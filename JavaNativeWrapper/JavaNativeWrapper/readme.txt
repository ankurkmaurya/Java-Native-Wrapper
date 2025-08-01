========================================================================
    JavaNativeWrapper Project Overview
========================================================================

  Java Native Wrapper

  It wraps the java compiled binaries like .class files or .jar file 
  into executable windows binaries so that the java executables can 
  have the functionalities like other native window binaries.
  Java binaries wrapped though this utility can be made to run as a 
  windows service or can be seen as a executable with specific name
  in the task manager tool.
  It uses the Java Native Interface (JNI) feature of the Java to
  load and run the java compiled binaries through program or utility
  written in native/system dependent code.

  This utility just loads and runs the already installed jvm binaries
  through native code by first configuring the java binaries 
  (.jar or .class) through the configurable file 'config' which is 
  required to run this utility.

  Following are the required prerequisites for this utility
  1. Configuration File - config
     It requires a 'config' file in the same directory as this utility is 
     started. 'config' file can we used to provide various parameters to 
     the JVM and the application to be running.

     Below are the properties that can be configured
     a. java.home

     b. java.class.file.path

     c. jar.file.path

     d. main.class

     e. main.method.arguments


  2. Java Installation
     This utility when started tries to find the Java installation directory
     configured through the system environment variables. It checks the 
     'JAVA_HOME', 'JRE_HOME' and 'path' environment variables for Java 
     installation directory.
     By default the 'java.home' property in the config file is checked first 
     for Java installation, if you require to provide any other path for the
     Java binaries just provide the path in the configuration file otherwise 
     keep the value field blank.

========================================================================
Learn more about Java Native Interface here:
https://docs.oracle.com/en/java/javase/21/docs/specs/jni/index.html
========================================================================
