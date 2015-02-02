# Cinder Equalizer

An Audio Equalizer built with [Cinder](http://libcinder.org/).

--------------------
### To get Cinder cloned:

    git submodule foreach git pull

### To get Cinder dependencies cloned:

    cd thirdparty/cinder
    git submodule update --init --recursive

### To build Cinder:

Navigate to `thirdparty/cinder` and open up the appropriate Visual Studio solution folder (`vc2013` for Visual Studio 2013 for example) and build Cinder in it under **both Debug and Release** modes.

### To build this project:

Use [CMake](http://www.cmake.org/) to generate the Solution files. Either use CMake GUI or use `cmake .. -G"Visual Studio 12"` in `build` folder. Open up `ciEq.sln` and build the project afterwards.