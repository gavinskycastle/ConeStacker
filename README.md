# Cone Stacker

![Native Build](https://github.com/gavinskycastle/ConeStacker/actions/workflows/nativebuild.yml/badge.svg) ![Web Build](https://github.com/gavinskycastle/ConeStacker/actions/workflows/webbuild.yml/badge.svg)

## Building to Web

**You need to install Emscripten** [Download Here!](https://emscripten.org/docs/getting_started/downloads.html)

1. Clone the repository using Git. Execute this in a terminal: `git clone https://github.com/gavinskycastle/ConeStacker.git`
1. Cd into ConeStacker. `cd ConeStacker`
1. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
1. Make a build folder. Your builds will go here. `mkdir build; cd build`
1. Setup cmake `emcmake cmake -S .. -D CMAKE_BUILD_TYPE=Release`
1. Run `cmake --build build` to compile the project
1. Run a local web server and open the ConeStacker.html

## Building (Linux/macOS)
 
1. Clone the repository using `git clone https://github.com/gavinskycastle/ConeStacker.git`
1. Move to the ConeStacker directory `cd ConeStacker`
1. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
1. Install the required build tools (for example on Arch, type `sudo pacman -S cmake make gcc`)
1. Make a build folder and cd to it with `mkdir build && cd build`
1. Setup cmake `cmake .. -DCMAKE_BUILD_TYPE=Release`
1. Run `make` to compile the project
1. Make the binary executable `chmod +x ConeStacker`
1. Run the binary with `./ConeStacker`

## Building (Windows, MinGW)

**You need to install MinGW. Select make and developer tools in the installer** [Download MinGW here!](https://sourceforge.net/projects/mingw/)

1. Clone the repository using Git. Execute this in a terminal: `git clone https://github.com/gavinskycastle/ConeStacker.git`
1. Cd into ConeStacker. `cd ConeStacker`
1. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
1. Make a build folder. Your builds will go here. `mkdir build; cd build`
1. Setup CMake. **Make sure to define MinGW Makefiles if you are using MinGW!** `cmake .. -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"`
1. Make sure you have mingw32-make. If you do, just run it in the build directory and it will start compiling everything. It won't take long, and once it's finished, then
1. Run ConeStacker.exe! You have just compiled ConeStacker for Windows using MinGW.

## Building (Windows, MSVC)

You need to have Visual Studio 2019 (other versions aren't tested, please tell us if it works for you) with C++ Development selected in the installer.

1. Clone the repository using Git. Execute this in a terminal: `git clone https://github.com/gavinskycastle/ConeStacker.git`
1. Cd into ConeStacker. `cd ConeStacker`
1. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
1. Make a build folder. Your builds will go here. `mkdir build; cd build`
1. Setup CMake. `cmake .. -DCMAKE_BUILD_TYPE=Release`
1. Let's build the project! Run `cmake --build .`
1. Go into Debug, your build of ConeStacker is there. You have now compiled ConeStacker for Windows using MSVC.
