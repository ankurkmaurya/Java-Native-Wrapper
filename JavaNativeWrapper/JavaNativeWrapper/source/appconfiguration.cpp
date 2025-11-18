
#include "pch.h"
#include "appconfiguration.h"
#include "logger.h"


const std::string AppConfiguration::configFileExtension = CONFIG_FILE_EXTENSION;
const std::string AppConfiguration::keyJNWEnableSysOutLog = JNW_ENABLE_SYS_OUT_LOG;
const std::string AppConfiguration::keyJNWRunWithAdminPrivelege = JNW_RUN_WITH_ADMIN_PRIVILEGE;
const std::string AppConfiguration::keyJNWServiceName = JNW_SERVICE_NAME;
const std::string AppConfiguration::keyJNWServiceNameDisplay = JNW_SERVICE_NAME_DISPLAY;
const std::string AppConfiguration::keyJNWServiceDescription = JNW_SERVICE_DESCRIPTION;
const std::string AppConfiguration::keyJavaHome = JAVA_HOME;
const std::string AppConfiguration::keyJavaVMArgs = JAVA_VM_ARGUMENTS;
const std::string AppConfiguration::keyMainClass = MAIN_CLASS;
const std::string AppConfiguration::keyMainMethodArguments = MAIN_METHOD_ARGUMENTS;
const std::string AppConfiguration::keyJavaClassFilePath = JAVA_CLASS_FILE_PATH;
const std::string AppConfiguration::keyJarFilePath = JAR_FILE_PATH;


std::string AppConfiguration::configFileName = CONFIG_FILE_NAME;
bool AppConfiguration::enableJNWSysOutLog = false;
bool AppConfiguration::runJNWWithAdminPrivilege = false;
bool AppConfiguration::showCoutConsole = false;
std::string AppConfiguration::jnwServiceName = "";
std::string AppConfiguration::jnwServiceNameDisplay = "";
std::string AppConfiguration::jnwServiceDescription = "";
std::string AppConfiguration::jvmdllPath = "";
std::string AppConfiguration::javaVMArguments = "";
std::string AppConfiguration::javaBinaryPath = "";
std::string AppConfiguration::mainClassPath = "";
std::string AppConfiguration::mainMethodArguments = "";


bool AppConfiguration::getEnableJNWSysOutLog() { return AppConfiguration::enableJNWSysOutLog; }
bool AppConfiguration::getRunJNWWithAdminPrivilege() { return AppConfiguration::runJNWWithAdminPrivilege; }
bool AppConfiguration::getShowCoutConsole() { return AppConfiguration::showCoutConsole; }
std::string AppConfiguration::getConfigFileExtension() { return AppConfiguration::configFileExtension; }
std::string AppConfiguration::getJnwServiceName() { return AppConfiguration::jnwServiceName; }
std::string AppConfiguration::getJnwServiceNameDisplay() { return AppConfiguration::jnwServiceNameDisplay; }
std::string AppConfiguration::getJnwServiceDescription() { return AppConfiguration::jnwServiceDescription; }
std::string AppConfiguration::getJvmdllPath() { return AppConfiguration::jvmdllPath; }
std::string AppConfiguration::getJavaVMArguments() { return AppConfiguration::javaVMArguments; }
std::string AppConfiguration::getJavaBinaryPath() { return AppConfiguration::javaBinaryPath; }
std::string AppConfiguration::getMainClassPath() { return AppConfiguration::mainClassPath; }
std::string AppConfiguration::getMainMethodArguments() { return AppConfiguration::mainMethodArguments; }


void AppConfiguration::setShowCoutConsole(bool showCoutConsole) {
    AppConfiguration::showCoutConsole = showCoutConsole;  
}

void AppConfiguration::setEnableJNWSysOutLog(bool enableJNWSysOutLog) {
    AppConfiguration::enableJNWSysOutLog = enableJNWSysOutLog;
}

void AppConfiguration::setRunJNWWithAdminPrivilege(bool runJNWWithAdminPrivilege) {
    AppConfiguration::runJNWWithAdminPrivilege = runJNWWithAdminPrivilege;
}

void AppConfiguration::setConfigFileName(std::string configFileName) {
    AppConfiguration::configFileName = configFileName;
}

void AppConfiguration::initializeAppConfiguration(bool log_enabled) {
    //jnw.enable.system.out.logging
    std::string jnwEnableSysOutLog = ConfigUtil::getConfigValue(configFileName, keyJNWEnableSysOutLog);
    if (jnwEnableSysOutLog.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'jnw.enable.system.out.logging' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'jnw.enable.system.out.logging' configuration found: {}", jnwEnableSysOutLog);
        if (jnwEnableSysOutLog.compare("yes") == 0 ||
            jnwEnableSysOutLog.compare("Yes") == 0 ||
            jnwEnableSysOutLog.compare("YES") == 0) {
            AppConfiguration::setEnableJNWSysOutLog(true);
        }
    }

    //jnw.run.with.admin.priviledge
    std::string jnwRunWithAdminPrivilege = ConfigUtil::getConfigValue(configFileName, keyJNWRunWithAdminPrivelege);
    if (jnwRunWithAdminPrivilege.compare("") == 0) {
        log_enabled&& Logger::log(LogLevel::WARN, "'jnw.run.with.admin.priviledge' configuration not found");
    }
    else {
        log_enabled&& Logger::log(LogLevel::INFO, "'jnw.run.with.admin.priviledge' configuration found: {}", jnwRunWithAdminPrivilege);
        if (jnwRunWithAdminPrivilege.compare("yes") == 0 ||
            jnwRunWithAdminPrivilege.compare("Yes") == 0 ||
            jnwRunWithAdminPrivilege.compare("YES") == 0) {
            AppConfiguration::setRunJNWWithAdminPrivilege(true);
        }
    }

    //jnw.service.name
    AppConfiguration::jnwServiceName = ConfigUtil::getConfigValue(configFileName, keyJNWServiceName);
    if (AppConfiguration::jnwServiceName.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'jnw.service.name' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'jnw.service.name' configuration found: {}", AppConfiguration::jnwServiceName);
    }

    //jnw.service.name.display
    AppConfiguration::jnwServiceNameDisplay = ConfigUtil::getConfigValue(configFileName, keyJNWServiceNameDisplay);
    if (AppConfiguration::jnwServiceNameDisplay.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'jnw.service.name.display' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'jnw.service.name.display' configuration found: {}", AppConfiguration::jnwServiceNameDisplay);
    }

    //jnw.service.description
    AppConfiguration::jnwServiceDescription = ConfigUtil::getConfigValue(configFileName, keyJNWServiceDescription);
    if (AppConfiguration::jnwServiceDescription.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'jnw.service.description' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'jnw.service.description' configuration found: {}", AppConfiguration::jnwServiceDescription);
    }

    //java.home
    std::string valueJavaHome = ConfigUtil::getConfigValue(configFileName, keyJavaHome);
    if (valueJavaHome.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'java.home' configuration not found");
        AppConfiguration::jvmdllPath = AppUtil::findJVMDLLPath(log_enabled);
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'java.home' configuration found, Value: {}", valueJavaHome);
        AppConfiguration::jvmdllPath = AppUtil::checkJVMDLLFileExists(valueJavaHome);
    }
    if (AppConfiguration::jvmdllPath.empty()) {
        Logger::log(LogLevel::ERR, "'jvm.dll' installation not found on the system.");
        if (AppConfiguration::getShowCoutConsole()) {
            system("pause");
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
    log_enabled && Logger::log(LogLevel::INFO, "'jvm.dll' found on path: {}", AppConfiguration::jvmdllPath);

    //java.vm.arguments
    AppConfiguration::javaVMArguments = ConfigUtil::getConfigValue(configFileName, keyJavaVMArgs);
    if (AppConfiguration::javaVMArguments.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'java.vm.arguments' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'java.vm.arguments' configuration found: {}", AppConfiguration::javaVMArguments);
    }

    //java.class.file.path
    std::string javaClassFilePath = ConfigUtil::getConfigValue(configFileName, keyJavaClassFilePath);
    if (javaClassFilePath.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'java.class.file.path' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'java.class.file.path' configuration found: {}", javaClassFilePath);
        AppConfiguration::javaBinaryPath = javaClassFilePath;
    }

    //jar.file.path
    std::string jarFilePath = ConfigUtil::getConfigValue(configFileName, keyJarFilePath);
    if (jarFilePath.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'jar.file.path' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'jar.file.path' configuration found: {}", jarFilePath);
        AppConfiguration::javaBinaryPath = jarFilePath;
    }
    if (AppConfiguration::javaBinaryPath.empty()) {
        Logger::log(LogLevel::ERR, "No Java binary has been configured.");
        if (AppConfiguration::getShowCoutConsole()) {
            system("pause");
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
    log_enabled && Logger::log(LogLevel::INFO, "Java binary to be executed: {}", AppConfiguration::javaBinaryPath);

    //main.class
    std::string mainClass = ConfigUtil::getConfigValue(configFileName, keyMainClass);
    if (mainClass.compare("") == 0) {
        Logger::log(LogLevel::ERR, "'main.class' configuration not found");
        if (AppConfiguration::getShowCoutConsole()) {
            system("pause");
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
    log_enabled && Logger::log(LogLevel::INFO, "'main.class' configuration found, Value: {}", mainClass);
    AppConfiguration::mainClassPath = StringUtil::replaceAll(mainClass, std::string("."), std::string("/"));
    log_enabled && Logger::log(LogLevel::INFO, "MainClass path : {}", AppConfiguration::mainClassPath);

    //main.method.arguments
    AppConfiguration::mainMethodArguments = ConfigUtil::getConfigValue(configFileName, keyMainMethodArguments);
    if (AppConfiguration::mainMethodArguments.compare("") == 0) {
        log_enabled && Logger::log(LogLevel::WARN, "'main.method.arguments' configuration not found");
    }
    else {
        log_enabled && Logger::log(LogLevel::INFO, "'main.method.arguments' configuration found : {}", AppConfiguration::mainMethodArguments);
    }
}





