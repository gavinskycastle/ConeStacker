# Cone Stacker 2

![Native Build](https://github.com/gavinskycastle/ConeStacker2/actions/workflows/nativebuild.yml/badge.svg) ![Web Build](https://github.com/gavinskycastle/ConeStacker2/actions/workflows/webbuild.yml/badge.svg)

Build system and instructions copied from [SasLuca/raylib-cpp-cmake-template](https://github.com/SasLuca/raylib-cpp-cmake-template)

## Building to Web

**You need to install Emscripten** [Download Here!](https://emscripten.org/docs/getting_started/downloads.html)

1. Clone the repository using Git. Execute this in a terminal: `git clone https://github.com/gavinskycastle/ConeStacker2.git`
2. Cd into ConeStacker2. `cd ConeStacker2`
3. Install the required build tools (for example on Arch, type `sudo pacman -S cmake make gcc pkg-config wayland-protocols`)
4. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
5. Make a build folder. Your builds will go here. `mkdir build; cd build`
6. Setup cmake `emcmake cmake -S .. -D CMAKE_BUILD_TYPE=Release`
7. Run `cmake --build build` to compile the project
8. Run a local web server and open the ConeStacker2.html

## Building (Linux/macOS)
 
1. Clone the repository using `git clone https://github.com/gavinskycastle/ConeStacker2.git`
2. Move to the ConeStacker2 directory `cd ConeStacker2`
3. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
4. Install the required build tools (for example on Arch, type `sudo pacman -S cmake make gcc`)
5. Make a build folder and cd to it with `mkdir build && cd build`
6. Setup cmake `cmake .. -DCMAKE_BUILD_TYPE=Release`
7. Run `make` to compile the project
8. Make the binary executable `chmod +x ConeStacker2`
9. Run the binary with `./ConeStacker2`

## Building (Windows, MinGW-w64)

**You need to install MinGW-w64. Copy the folder and add C:\mingw64\bin to PATH** [Download MinGW-w64 here!](https://github.com/niXman/mingw-builds-binaries/releases)

1. Clone the repository using Git. Execute this in a terminal: `git clone https://github.com/gavinskycastle/ConeStacker2.git`
2. Cd into ConeStacker2. `cd ConeStacker2`
3. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
4. Make a build folder. Your builds will go here. `mkdir build; cd build`
5. Setup CMake. **Make sure to define MinGW Makefiles if you are using MinGW-w64!** `cmake .. -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"` **You might also want to [set the default generator to MinGW.](https://stackoverflow.com/a/76580126/13137729)**
6. Make sure you have mingw32-make. If you do, just run it in the build directory and it will start compiling everything. It won't take long, and once it's finished, then
7. Run ConeStacker2.exe! You have just compiled ConeStacker2 for Windows using MinGW.

## Building (Windows, MSVC)

You need to have Visual Studio 2019 (other versions aren't tested, please tell us if it works for you) with C++ Development selected in the installer.

1. Clone the repository using Git. Execute this in a terminal: `git clone https://github.com/gavinskycastle/ConeStacker2.git`
2. Cd into ConeStacker2. `cd ConeStacker2`
3. Install the raylib git submodule using `git submodule update --init --recursive --depth=1`
4. Make a build folder. Your builds will go here. `mkdir build; cd build`
5. Setup CMake. `cmake .. -DCMAKE_BUILD_TYPE=Release`
6. Let's build the project! Run `cmake --build .`
7. Go into Debug, your build of ConeStacker2 is there. You have now compiled ConeStacker2 for Windows using MSVC.
