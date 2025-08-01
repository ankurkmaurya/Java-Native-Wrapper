
#include "pch.h"

#include <windows.h>
#include <winnt.h>

#include <stdio.h>
#include <jni.h>
#include <string>

#include <iostream>
#include <fstream>

#include "logger.h"
#include "stringutility.h"
#include "fileutility.h"
#include "javavirtualmachine.h"
#include "jvmconsoleredirector.h"


JavaVM* JavaVirtualMachine::jvm = nullptr;
JNIEnv* JavaVirtualMachine::jenv = nullptr;


void callJVMConsoleRedirector(JNIEnv* env, const char* logFilePath, bool log_enabled) {
    jclass cls = env->DefineClass(
        "JVMConsoleRedirector",  // class name
        NULL,                    // loader (bootstrap)
        reinterpret_cast<const jbyte*>(JVMConsoleRedirector_class),
        JVMConsoleRedirector_class_len
    );

    if (!cls) {
        Logger::log(LogLevel::ERR, "Failed to inject JVMConsoleRedirector class");
        return;
    }

    jmethodID mid = env->GetStaticMethodID(cls, "redirect", "(Ljava/lang/String;)V");
    if (!mid) {
        Logger::log(LogLevel::ERR, "Failed to find redirect method");
        return;
    }

    jstring logPath = env->NewStringUTF(logFilePath);
    env->CallStaticVoidMethod(cls, mid, logPath);
    log_enabled && Logger::log(LogLevel::INFO, "System.out print redirected to log file");
}


bool static loadJvmDll(JavaVirtualMachine::CreateJavaVM* createJavaVM, 
                  std::string const& jvmDLLPath,
                  bool log_enabled) {

    log_enabled && Logger::log(LogLevel::INFO, "Loading JVM library from {}", jvmDLLPath);
    HINSTANCE jvmDll = LoadLibraryA(jvmDLLPath.c_str());
    if (jvmDll == nullptr) {
        Logger::log(LogLevel::ERR, "Failed to load jvm.dll");
        DWORD lastErrorCode = GetLastError();
        Logger::log(LogLevel::ERR, "Code [{}]", lastErrorCode);
        return false;
    }

    *createJavaVM = (JavaVirtualMachine::CreateJavaVM)GetProcAddress(jvmDll, "JNI_CreateJavaVM");
    log_enabled && Logger::log(LogLevel::INFO, "Loaded jvm.dll :)");
    return true;
}


bool static createJVM(JavaVM*& jvm, JNIEnv*& env,
    std::string const& jvmDLLPath,
    std::string const& javaBinaryPath,
    std::string const& javaVMArguments,
    bool log_enabled) {

    log_enabled&& Logger::log(LogLevel::INFO, "Loading jvm.dll .....");
    JavaVirtualMachine::CreateJavaVM createJavaVM;
    if (!loadJvmDll(&createJavaVM, jvmDLLPath, log_enabled)) {
        Logger::log(LogLevel::ERR, "Failed to load jvm.dll :(");
        return false;
    }

    /*
     * https://docs.oracle.com/javase/10/docs/specs/jni/invocation.html
     */
    log_enabled && Logger::log(LogLevel::INFO, "Creating JVM Instance");
    size_t vm_option_size = 2;
    // Create an argument array for the JVM runtime
    std::vector<std::string> jvm_args = StringUtil::split(javaVMArguments, ';');
    if (jvm_args.size() > 0) {
        vm_option_size = vm_option_size + jvm_args.size();
    }

    JavaVMOption* options = new JavaVMOption[vm_option_size];
    /*
     * set user classes
     */
    std::string javaClassPathOpt = "-Djava.class.path=" + javaBinaryPath;
    options[0].optionString = javaClassPathOpt.data();

    /*
     * set native library path
     */
    options[1].optionString = (char*)"-Djava.library.path=";

    for (int i = 2; i < vm_option_size; i++) {
        int jvmArgCnt = i - 2;
        options[i].optionString = jvm_args[jvmArgCnt].data();
    }

    if (log_enabled) {
        log_enabled && Logger::log(LogLevel::INFO, "Creating JVM Instance with following Option.");
        for (int i = 0; i < vm_option_size; i++) {
            JavaVMOption optio = options[i];
            std::ostringstream oss;
            oss << (i + 1) << " : " << optio.optionString;
            log_enabled && Logger::log(LogLevel::INFO, oss.str());
        }
    }
    

    JavaVMInitArgs vm_args;
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = vm_option_size;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = true;

    log_enabled && Logger::log(LogLevel::INFO, "Starting JVM....");

    jint creationRslt = createJavaVM(&jvm, (void**)&env, &vm_args);
    if (creationRslt == JNI_OK) {
        log_enabled && Logger::log(LogLevel::INFO, "Loading JVM library from {}", jvmDLLPath);
        log_enabled && Logger::log(LogLevel::INFO, "JVM started");
        delete[] options;
        return true;
    }

    if (creationRslt == JNI_ERR) {
        Logger::log(LogLevel::ERR, "JVM failed to start : unknown error");
    }
    if (creationRslt == JNI_EVERSION) {
        Logger::log(LogLevel::ERR, "JVM failed to start : JNI version error");

        char jvmVersionHexStr[10];
        sprintf(jvmVersionHexStr, "0x%x", vm_args.version);
        Logger::log(LogLevel::ERR, "Required Version: {}", jvmVersionHexStr);
    }
    if (creationRslt == JNI_ENOMEM) {
        Logger::log(LogLevel::ERR, "JVM failed to start : not enough memory");
    }
    if (creationRslt == JNI_EEXIST) {
        Logger::log(LogLevel::ERR, "JVM failed to start : VM already created");
    }
    if (creationRslt == JNI_EINVAL) {
        Logger::log(LogLevel::ERR, "JVM failed to start : invalid arguments");
    }
    if (jvm == NULL) {
        Logger::log(LogLevel::ERR, "JVM failed to start.");
    }
    delete[] options;
    return false;
}


void static runMainClass(JavaVM* jvm, JNIEnv* env,
    std::string const& mainClassPath,
    std::string const& mainMethodArguments) {

    // Find the main class
    jclass cls = env->FindClass(mainClassPath.data());
    if (cls == NULL) {
        Logger::log(LogLevel::ERR, "Unable to find class: {}", mainClassPath);
        exit(EXIT_FAILURE);
    }

    // Find the main method
    jmethodID mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
    if (mid == NULL) {
        Logger::log(LogLevel::ERR, "Unable to find main method in class: {}", mainClassPath);
        exit(EXIT_FAILURE);
    }

    //Create an argument array for the main method
    std::vector<std::string> main_method_args = StringUtil::split(mainMethodArguments, ' ');
    jobjectArray mainArgs = env->NewObjectArray((jsize)main_method_args.size(), env->FindClass("java/lang/String"), NULL);
    for (size_t i = 0; i < main_method_args.size(); ++i) {
        std::string main_method_arg = main_method_args[i];
        // Create and set the arguments
        jstring argStr = env->NewStringUTF(main_method_arg.data());
        env->SetObjectArrayElement(mainArgs, (jsize)i, argStr);
        env->DeleteLocalRef(argStr);
    }

    // Call the main method
    env->CallStaticVoidMethod(cls, mid, mainArgs);
}


void JavaVirtualMachine::startJavaVirtualMachine(bool log_enabled) {
    //Start the JVM instance
    if (!createJVM(JavaVirtualMachine::jvm, JavaVirtualMachine::jenv,
        AppConfiguration::getJvmdllPath(),
        AppConfiguration::getJavaBinaryPath(),
        AppConfiguration::getJavaVMArguments(),
        log_enabled)) {
        exit(EXIT_FAILURE);
    }

    if (AppConfiguration::getEnableJNWSysOutLog()) {
        //First call the Java Console Redirector Class to redirect system.out console print lines to log file
        callJVMConsoleRedirector(JavaVirtualMachine::jenv, FileUtil::consoleOutLogFile.c_str(), log_enabled);
    }

    log_enabled && Logger::log(LogLevel::INFO, "Running Main Class");
    runMainClass(jvm, jenv,
        AppConfiguration::getMainClassPath(),
        AppConfiguration::getMainMethodArguments());
    jvm->DestroyJavaVM();
}

