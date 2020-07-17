Support development of Runtime Compiled C++ through [Github Sponsors](https://github.com/sponsors/dougbinks) or our [Patreon](https://www.patreon.com/enkisoftware)

[<img src="https://img.shields.io/static/v1?logo=github&label=Github&message=Sponsor&color=#ea4aaa" width="200"/>](https://github.com/sponsors/dougbinks)    [<img src="https://c5.patreon.com/external/logo/become_a_patron_button@2x.png" alt="Become a Patron" width="150"/>](https://www.patreon.com/enkisoftware)

# Runtime Compiled C++ sample code

- Wiki: https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/wiki
- Blog: https://www.enkisoftware.com/devlog-rcc++ (Old Blog at: http://runtimecompiledcplusplus.blogspot.com/)
- Group: http://groups.google.com/group/runtimecompiledcplusplus
- Code: https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus
- Further examples:
    - https://github.com/juliettef/RCCpp-DearImGui-GLFW-example (Cross platform)
    - https://github.com/dougbinks/RCCpp_DX11_Example (Windows only)

Runtime-Compiled C++ (RCC++) is a way to reliably make major changes to your C++ code at runtime and see the results immediately. It's aimed at games development but could be useful in any industry where turnaround times are a bottleneck.

RCC++ is primarily designed to shorten iteration times in development - developers can build their project, run it, make changes during runtime and see the results almost immediately. If needed, shipping code can [disable runtime compilation](https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/wiki/Disabling-runtime-compilation) in a number of ways. RCC++ is not intended as a method to allow end users of a shipped binary to compile modifications, though with some work it can be used this way.

## Features

- Cross platform, supporting MSVC on Windows, Clang and GCC or any compiler which understands GCC options on Linux and Mac OSX, and relatively easy to port to new compilers and platforms.
- Simple in memory serialization so you can preserve object state in a safe way between compiles.
- [Error protection](https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/wiki/Error-protection) so that when you make a mistake during programming which would normally crash your application you can correct and recover.
- [Undo and Redo](https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/wiki/Undo-and-Redo-via-the-Object-Constructor-History) which allows you to quickly swap between changes at runtime, great for testing whether a subtle code change helps.
- [Control over optimization levels](https://github.com/RuntimeCompiledCPlusPlus/RuntimeCompiledCPlusPlus/wiki/Controlling-Optimization-Levels) so you can switch one file into debug, add a break point and see the state in more clarity than when it's fully optimized.

## Supported OS / Compilers:

- Linux, Windows, Mac OS X suported through cmake files (many thanks to user join_the_fun from reddit)
- Visual Studio projects for Windows are also provided.
- MinGW is not supported in this repository, but a port exists here: https://github.com/BobSmun/RuntimeCompiledCPlusPlus/tree/MinGWw64_Support

For Visual Studio the main project file is found in the Aurora directory. All dependencies should be normally present.

Linux requires the following dependencies installed for the SimpleTest project (use "sudo apt-get install NAME"):
- libfreetype6-dev
- libx11-dev
- libgl1-mesa-dev
- libglu1-mesa-dev
- g++, if you're already doing C++ development you should have this
- libglfw-dev, if using system glfw, otherwise this comes prebuilt for 64bit Linux. To use the system glfw set the option GLFW_SYSTEM to ON - cmake .. -DGLFW_SYSTEM=ON

For cmake, create a folder called build in the Aurora directory and run cmake from there followed by make: on Linux run "mkdir build && cd build && cmake .. && make" from Aurora dir.

## License (zlib)

Copyright (c) 2010-2020 Matthew Jack and Doug Binks

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
