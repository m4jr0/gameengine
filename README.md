# Comet Game Engine

## Description

The **Comet Game Engine** is an experimental project that I'm developing in my free time to explore and learn about game engine programming in a practical, hands-on way.

### Useful Links

* [**Quick Overview**](docs/QUICK_OVERVIEW.md): Short technical presentation of **Comet**. How it works, what it does, and why I built it. _If you only read one thing, make it this one!_
* [Development Notes](docs/DEV_NOTES.md): Code style, TODO format, and development conventions.  
* [Asset Credits](docs/ASSET_CREDITS.md): Attributions for all third-party assets.  
* [Resources & Inspirations](docs/REFERENCES.md): Talks, books, and articles that inspired **Comet** (and me) along the way. _I was kidding above: **this** is the real gold. You'll learn a lot!_

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

## Current Goals

* Animation blending
* Skyboxes, lights & shadows
* Basic physics system
* Remove GLM and STL in favor of custom math
* UI system
* Scripting system
* ECS iteration improvements
* Generic resource system
* Support transparency
* Add occlusion culling

## External Libraries

**Comet** tries to minimize dependencies, but a few key libraries are used to speed up development and handle platform details.

### Engine

- [GLFW](https://www.glfw.org/): windowing and input abstraction  
- [GLAD](https://github.com/Dav1dde/glad): OpenGL loader  
- [Vulkan Memory Allocator (VMA)](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator): Vulkan memory management  
- [GLM](https://github.com/g-truc/glm): temporary math library (planned to be replaced by custom math types)  
- [Dear ImGui](https://github.com/ocornut/imgui): debug UI and profiling tools  
- [lz4](https://github.com/lz4/lz4): resource compression  

### Editor

- [Assimp](https://github.com/assimp/assimp): model importing  
- [stb](https://github.com/nothings/stb): texture loading  
- [JSON for Modern C++ (nlohmann)](https://github.com/nlohmann/json): configuration and metadata  
- [Shaderc](https://github.com/google/shaderc): shader compilation

## License

This project is under the [MIT license](https://github.com/m4jr0/gameengine/blob/master/LICENSE).
