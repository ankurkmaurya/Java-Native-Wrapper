#pragma once

#include "pch.h"
#include "fileutility.h"
#include "stringutility.h"
#include "configutility.h"
#include "apputility.h"

#include <string>



#define CONFIG_FILE_NAME         "config"
#define CONFIG_FILE_EXTENSION    ".config"

#define JNW_ENABLE_SYS_OUT_LOG         "jnw.enable.system.out.logging="
#define JNW_ENABLE_CONSOLE             "jnw.enable.console="
#define JNW_RUN_WITH_ADMIN_PRIVILEGE   "jnw.run.with.admin.privilege="

#define JNW_SERVICE_NAME          "jnw.service.name="
#define JNW_SERVICE_NAME_DISPLAY  "jnw.service.name.display="
#define JNW_SERVICE_DESCRIPTION   "jnw.service.description="

#define JAVA_HOME                 "java.home="
#define JAVA_VM_ARGUMENTS         "java.vm.arguments="
#define MAIN_CLASS                "main.class="
#define MAIN_METHOD_ARGUMENTS     "main.method.arguments="
#define JAVA_CLASS_FILE_PATH      "java.class.file.path="
#define JAR_FILE_PATH             "jar.file.path="



class AppConfiguration
{
private:
    
    static const std::string configFileExtension;
    static const std::string keyJNWEnableSysOutLog;
    static const std::string keyJNWEnableConsole;
    static const std::string keyJNWRunWithAdminPrivelege;
    static const std::string keyJNWServiceName;
    static const std::string keyJNWServiceNameDisplay;
    static const std::string keyJNWServiceDescription;
    static const std::string keyJavaHome;
    static const std::string keyJavaVMArgs;
    static const std::string keyMainClass;
    static const std::string keyMainMethodArguments;
    static const std::string keyJavaClassFilePath;
    static const std::string keyJarFilePath;

    static std::string configFileName;
    static bool enableJNWSysOutLog;
    static bool enableJNWConsole;
    static bool runJNWWithAdminPrivilege;
    static bool showCoutConsole;
    static std::string jnwServiceName;
    static std::string jnwServiceNameDisplay;
    static std::string jnwServiceDescription;
    static std::string jvmdllPath;
    static std::string javaVMArguments;
    static std::string javaBinaryPath;
    static std::string mainClassPath;
    static std::string mainMethodArguments;

public:
    static bool getEnableJNWSysOutLog();
    static bool getEnableJNWConsole();
    static bool getRunJNWWithAdminPrivilege();
    static bool getShowCoutConsole();
    static std::string getConfigFileExtension();

    static std::string getJnwServiceName();
    static std::string getJnwServiceNameDisplay();
    static std::string getJnwServiceDescription();
    static std::string getJvmdllPath();
    static std::string getJavaVMArguments();
    static std::string getJavaBinaryPath();
    static std::string getMainClassPath();
    static std::string getMainMethodArguments();

    static void initializeAppConfiguration(bool log_enabled);
    static void setShowCoutConsole(bool showCoutConsole);
    static void setEnableJNWSysOutLog(bool enableJNWSysOutLog);
    static void setRunJNWWithAdminPrivilege(bool runJNWWithAdminPrivilege);
    static void setEnableJNWConsole(bool enableJNWConsole);
    static void setConfigFileName(std::string configFileName);

};



