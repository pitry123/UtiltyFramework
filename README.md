# Introduction 
Framework infrastructure code version 2.0 for LandSystems RT Embedded projects.
- Cross platform (Windows/Linux)
- Coded in C++ 11
- CMake based project


Refer to [Wiki Page](http://gitlab/LandRT/ezFramework2/Infrastructure/wikis/home) for more information of how to develop on ezFrmaework 2.0

[![Build Status](http://jenkins/buildStatus/icon?job=Landc4i_Dev%2FLAND%2FFramework2.0%2FInfrastructure)](http://jenkins/job/Landc4i_Dev/job/LAND/job/Framework2.0/job/Infrastructure/)
# Getting Started
## Installation Instructions:
###  On Windows
1. Make sure Visual Studio 2017 is installed, updated with latest updates and supports CMake for C++ 
   (To update Visual Studio 2017 or modify it's installation you need to run Visual Studio Installer). 
   Visual Studio 2017 come with CMake as part of it, so it does not need to be installed seperatlly. 
2. if not using Conan:
   1. create a folder in which you will hold all source code and third party dependencies for Framework 2.0 
   (For example `J:\LandSystems\MasterTrunk`)
   2. Define windows environment variable called `DEVELOPMENT_ROOT` to point to this location
   3. Create folder named **Framework_2.0** to hold source code (from repository to clone)
5. Clone the following [Framework 2.0 Infrastructure Git Repository](http://tfs1:8080/tfs/LandC4ICollection/Framework_2.0/_git/Infrastructure or from gitlab http://gitlab/LandRT/ezFramework2/Infrastructure for the innersource version) to get source code

If using conan (cmake flag USE_CONAN=ON) [Click here](http://gitlab/LandRT/Genral/wikis/Installing-and-Creating-packages-using-conan-cpp-package-manager) for further explanation how to connect it to the project dependecies in the artifatory
This method assumes connectivity to [Elbit JFROG Artifactory](http://nteptartifact:8081/artifactory/webapp/#/artifacts/browse/tree/General/LAND_C4I-LandRT-conan)

###  On Linux Ubuntu 18.04
It is recomnded to use QT Creator as the IDE for Linux, but any IDE supporting CMake can be used (vscode, CLion etc.)
In linux most of the ThirdParties can be installed by sodu apt-get 
write: 
`sudo apt-get install cmake libboost1.65-all-dev libgstreamer-plugins-base1.0-dev libopencv-dev libglfw3-dev libglu1-mesa-dev`
`sudo apt-get  libgstrtspserver-1.0-dev`

### Required Packags Windows:
- If not using Conan (cmake flag USE_CONAN=OFF) install the following pacakges before compiling the source
	- During GStreamer installation, make sure it's installed in your development folder `DEVELOPMENT_ROOT` under a folder called `ThirdParty`
	- For GStreamer, you need to install both binaries and development tools (two separate installations)
	- Copy Boost, GLFW & OpenCV folders from the [following location](\\esl\dfs\LandTools\Guides & Installations\Framework 2.0\ThirdParty) to [ThirdParty]() folder under your development root folder 
In 


# Build and Test
- Now that everything is installed correctly, run Visual Studio 2017/QT Creator and open Infrastructure folder under your development root folder
- Visual Studio should run the CMake  successfully (you can choose whether you are interested in 32 bit (x86) or 64 bit (x64) version (debug or release) of the code inside the Visual Studio IDE)
- Right click the `CMakeLists.txt` file and Rebuild All
- If everything went well you should now be able to open the solution file created under [VSProject]() folder with Visual Studio and run the samples
- QT Creator select the required platform and path the where to build. Configure and select `Run CMake` in the QT Build menu
- Disble Optional Third Party pacakges - OpenCV and GSreamer are considered as optional packages it is compiled as part of the infrastructure but it is possible to disable it
  - Visual Studio 2017: On `CMakeSettings.json` Add in the relevant configuration in the `"cmakeCommandArgs":` `-DUSE_OPENCV=OFF -DUSE_GSTREAMER=OFF` this will disbale all modules using OpenCV and GStreamer 
  - QT Creator: on the Configuration add `-DUSE_OPENCV=OFF -DUSE_GSTREAMER=OFF`
