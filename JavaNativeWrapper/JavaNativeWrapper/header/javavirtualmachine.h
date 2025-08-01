#pragma once


#include <jni.h>
#include <string>

#include "appconfiguration.h"


class JavaVirtualMachine
{
  public:
    static JavaVM* jvm;
    static JNIEnv* jenv;

    typedef jint(JNICALL* CreateJavaVM)(JavaVM**, void**, void*);
    static void startJavaVirtualMachine(bool log_enabled);

};





