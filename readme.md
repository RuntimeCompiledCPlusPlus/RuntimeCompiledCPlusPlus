# Runtime Compiled C++ sample code

- Code: https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus
- Wiki: https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/wiki
- Blog: http://runtimecompiledcplusplus.blogspot.com/
- Group: http://groups.google.com/group/runtimecompiledcplusplus

Runtime-Compiled C++ is a way to reliably make major changes to your C++ code at runtime and see the results immediately. It's aimed at games development but could be useful in any industry where turnaround times are a bottleneck.

## Supported OS / Compilers:

- Windows XP+, Visual Studio 2008+. Note we currently distribute only the VS 2010 solution and projects.
- Mac OS X 10.7+ with XCode 4. **NOTE** The SimpleTest project has a graphical issue when built via the XCode project, please use cmake until this is resolved. See [Issue 84](https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/issues/84)
- Linux using Eclipse CDT (tested Ubuntu 12.04 64bit).
- cmake (many thanks to user join_the_fun from reddit)
- QtCreator using the cmake files. Open CMakeLists.txt in the Aurora directory. Make sure cmake is installed and on Windows the path to cmake is set in QtCreator->Tools->options->CMake
- MinGW is not supported in this repository, but a port exists here: https://github.com/BobSmun/RuntimeCompiledCPlusPlus/tree/MinGWw64_Support

For Visual Studio and XCode the main project file is found in the Aurora directory. All dependencies should be normally present.

Linux requires the following dependencies installed for the SimpleTest project (use "sudo apt-get install NAME"):
- libfreetype6-dev
- libx11-dev
- libgl1-mesa-dev
- libglu1-mesa-dev
- g++, if you're already doing C++ development you should have this
- libglfw-dev, if using system glfw, otherwise this comes prebuilt for 64bit Linux. To use the system glfw set the option GLFW_SYSTEM to ON - cmake .. -DGLFW_SYSTEM=ON

For Eclipse use File->Import->General->Existing Projects into Workspace and select the RuntimeCompiledCPlusPlus directory and import all projects it finds (best not to copy so you can keep everything up to date with git).

For cmake, create a folder called build in the Aurora directory and run cmake from there followed by make: on Linux run "mkdir build && cd build && cmake .. && make" from Aurora dir.

## License (zlib)

Copyright (c) 2010-2013 Matthew Jack and Doug Binks

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
