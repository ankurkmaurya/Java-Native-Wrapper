# Java-Native-Wrapper
It wraps the java compiled binaries like .class files or .jar file 
into executable windows binaries so that the java executables can 
have the functionalities like other native window binaries.

Java binaries wrapped though this utility can be made to run as a 
windows service or can be seen as a executable with specific name
in the task manager tool.

This utility just loads and runs the already installed jvm binaries
through native code by first configuring the java binaries 
(.jar or .class) through the configurable file '.config' which is 
required to run this utility.
