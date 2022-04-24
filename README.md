# Comet Game Engine

## Description

The **Comet Game Engine** is a simple software to help create video games in C++. Currently, Comet is being developed in my free time as an experimental project: to put it simply, it helps me learn game engine programming in an interesting way.

## Build the project

The project can be built on both Windows and Linux (it's been developed on/tested with **MSVC** and **GCC**, respectively), but because it is in active development, some manual steps might be required beforehand (like installing specific packages).

To handle its dependencies, **[Vcpkg](https://github.com/microsoft/vcpkg)** is used.

### With Visual Studio 2022

First, install the **C++ CMake tools for Windows** component via the **Visual Studio Installer**. Then, open the cloned project directly from **Visual Studio 2022**.

### With Visual Studio Code

Open the project with **Visual Studio Code**, and with **CTRL+SHIFT+P** execute the `CMake: Configure` command. Then, launch **Comet** with the **Comet (build) [Windows/Unix]** configuration.

**Note:** you will need to install the **[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)** and **[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)** extensions.

### With CMake only

On Linux, open a terminal and enter the following commands:
* `git clone https://github.com/m4jr0/gameengine.git`
* `cd gameengine && mkdir build && cd build`
* `cmake -DVCPKG_ROOT=/path/to/gameengine/vcpkg -DIS_VCPKG_CLONE=TRUE ..`
* `cmake --build .`

It should be pretty similar on Windows.

## License

This project is under the [MIT license](https://github.com/m4jr0/gameengine/blob/master/LICENSE).
