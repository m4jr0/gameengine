# Koma Game Engine

## Description

The **Koma Game Engine** is a simple software to help building video games in C++. Currently, KGE is being developed in my free time as an experimental project: to put it simply, it helps me learn game engine programming in an interesting way.

## Quick start

**Note:** you should follow these steps carefully, as some of them may be incorrect. Currently, the project is in an unstable/unfinished state, and these steps are only a guideline to help setting it up on another computer.

### 1. Install the required packages

First, install the required packages on your computer.

#### Windows 10

Install **[Windows 10 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/)** and **[CMake](https://cmake.org/download/)**.

#### Linux

You need to install the following packages:
* **Fedora**: `sudo dnf install mesa-libGL-devel libXxf86vm-devel libXinerama-devel libXcursor-devel libXrandr-devel libXi-devel libglew-dev perl cmake`
* **Ubuntu**: `sudo apt install libgl1-mesa-dev libxxf86vm-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev libxrandr-dev libxi-dev libglew-dev perl cmake`

These should be available on other Linux distributions as well.

### 2. Build the project

#### With Visual Studio

To use **Visual Studio** with **Vcpkg**, you have to set the `VCPKG_ROOT` environment variable to the desired location of your **Vcpkg directory**.

Then, after making sure via the **Visual Studio Installer** that the **C++ CMake tools for Windows** are correctly installed, open the cloned project directly on **Visual Studio**.

#### With Visual Studio Code

Open the project with **Visual Studio Code**, and with **CTRL+SHIFT+P** execute the `CMake: Configure` command. Then, launch **KGE** with the **KGE (build) [Windows/Unix]** configuration.

**Note:** you will need to install the **[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)** and **[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)** extensions. You might have to install the **[.NET SDK for Visual Studio Code](https://dotnet.microsoft.com/download/dotnet/sdk-for-vs-code?utm_source=vs-code&amp;utm_medium=referral&amp;utm_campaign=sdk-install)** as well.

#### With CMake only

Open a terminal and enter the following commands:
* `git clone https://github.com/m4jr0/gameengine.git`
* `cd gameengine && mkdir build && cd build`
* `cmake -DVCPKG_ROOT=/path/to/gameengine/vcpkg -DIS_VCPKG_CLONE=TRUE ..`
* `cmake --build .`

## License

This project is under the [MIT license](https://github.com/m4jr0/gameengine/blob/master/LICENSE).
