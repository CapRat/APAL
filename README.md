# APAL
APAL is an crossplatform audio abstraction plugin, which is currently under heavy development.
At the moment it supports VST2, VST3, LV2 and LADSPA. But there are a lot more to come, because its made for adding more Formats to it.
Because its in an early phase of development it just supports audio and midi processing ( sure a lot more too, but that are details).
There should be GUI and Parameter support soon, so that this Library can be used in production. 6

## Dependencies
Before building APAL make sure, that all dependencies are installed. For the buildprocess you need CMake and an compiler. 
To install optional dependencies like VST3, LV2 and some other test-utilities execute the install-script in the scripts folder.
Execute the script, which matches your plattform (sh for unix like operatingsystems and bat for windows). You can also just call 
the crossplattformway with cmake and execute the following commands in your terminal:
cd scripts
cmake -P  install.cmake
If you want to change the default installationbehaviour with parameters. The following variables are supported:
- BUILD_CONFIG    Type of build, which is passed to CMake. The Default is Release, but sth. like Debug works also.
- BUILD_DIR       Directory to clone/download stuff in and also build.  This is sth. like a temporary directory.
- INSTALL_PREFIX  This is the directory, where the dependencies are installed. (Unix-defaul: /usr/local  Windows:C:/Program Files)
- SKIP_VST3       Skips the installation of VST3. The default value is OFF.
- SKIP_TORTURE    Skips the installation of the TortureTester. The default value is OFF.
- SKIP_LV2        Skips the installation of LV2. The default value is OFF.
- SKIP_PLUGINVAL  Skips the installation of pluginval. The default value is OFF.

to change the variables in the command just add an -D before the script name.
So to change the BUILD_DIR and the INSTALL_PREFIX to a custom dependency directory in an insource build and call the script from the root project-folder the command would be:
cmake -DINSTALL_PREFIX=build/deps -DBUILD_DIR=build/deps -P scripts/install.cmake 


# Building
To Build APAL, you can just use the standard CMake behaviour (Make sure, that the required dependencies are installed). 
An Insource build from commandline, started in the root directory can look like:
mkdir build
cd build
cmake ..
cmake --build .

If you also want to install everything just execute the follwing command in the build directory with Administrator-permisson:
cmake --build . --target install
if you use Visual Studio with generated Project Files, this may fail. If it does execute the Solution as Administrator and execute the INSTALL-Project.

You can also just add APAL to your Repository and build it along with your Project. But make sure, that cmake finds the Modules, so the cmake functionality is also available for your Project.

# Using APAL
To use APAL you have to link the needed Libraries wiht your Plugin. You have to link the APAL-library and the needed format libraries.
If you want to build a Plugin, with support VST2,3 and LV2 then link your library with APAL vst2 vst3 and lv2 libraries.If you have seperated binaries for each format, than add your source multiple times and link APAL and just the formats you want to support in that plugin.

If you link properly you can than just derive from IPlugin and implement the whole components yourself, or just use LazyPlugin as a starting point, like in the examples.
The API should behave same, but there are some differences in the implementation. 
The examples are a good starting Point to get your Hands dirty, so look there if you want to know more.

# Thinks you may should know
# How to use APAL
APAL is build around an internal API, which unifies the audiopluginformats. To make the use of this API, there are a lot of things already implemeneted. 
The API consists of an IPlugin and the IPlugin give access to multiple components. But you dont have to implement them all. In the examples the LazyPlugin is used.
The LazyPlugin as an example just uses 3 already written components, which match this lets just get started approach. You can combine the components as you want, it could depent on your goal.
To get more information about the different implementation look in the Sourcecode documentation of the different classes. (The source-code generated doxygen comments are comming soon. Just be  a little patient.)

# About the internal API
This Information are usefull if you want to work with the API. If you just want to write cool Audioplugins, pick the right implementations and start programming your idea.
APAL is mainly an API, which unifies the audiopluginformats. So thats IPlugin. Because Audioplugin are quiet complicated right now, the functinality is seperated in components. 
IPlugin just manage all this components(... to be honest, in the most cases it just hold the references of the components).  
The portcomponent is an very important one, because it manages the in and outputdata like MIDI and audio, which is represented by an IPort interface.
The use of the direct portcomponent may not feel that great so... just use it like in the examples. Or do something fancy and share it with us. 
The other components are a featurecomponent, which allows formats to optimize codes and report problems, if a used feature is not available in the current format and an informationcomponent which very suprisingly holds information.
