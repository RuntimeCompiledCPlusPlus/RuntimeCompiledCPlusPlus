Requires Visual Studio 2008 dlls.
Release versions seems to be included with windows 7 as standard, but debug versions not.

Tried converting project to VS10 and recompiling, but got error below.
So, presumably it's not tested under that. Best steer clear.

For now am just using non-debug versions of rocketlib, which seems ok.
Grabbing those DLLs from another machine, or getting updated version of RocketLib, both possible solutions.


------ Build started: Project: RocketCore, Configuration: Debug Win32 ------
  TextureResource.cpp
C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\utility(163): error C2440: 'initializing' : cannot convert from 'int' to 'Rocket::Core::TextureHandle '
          Conversion from integral type to pointer type requires reinterpret_cast, C-style cast or function-style cast
          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\utility(247) : see reference to function template instantiation 'std::_Pair_base<_Ty1,_Ty2>::_Pair_base<_Ty,Rocket::Core::Vector2<Type>>(_Other1 &&,_Other2 &&)' being compiled
          with
          [
              _Ty1=Rocket::Core::TextureHandle ,
              _Ty2=Rocket::Core::Vector2i,
              _Ty=int,
              Type=int,
              _Other1=int,
              _Other2=Rocket::Core::Vector2<int>
          ]
          ..\Source\Core\TextureResource.cpp(164) : see reference to function template instantiation 'std::pair<_Ty1,_Ty2>::pair<int,Rocket::Core::Vector2<Type>>(_Other1 &&,_Other2 &&)' being compiled
          with
          [
              _Ty1=Rocket::Core::TextureHandle,
              _Ty2=Rocket::Core::Vector2i,
              Type=int,
              _Other1=int,
              _Other2=Rocket::Core::Vector2<int>
          ]
C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\utility(163): error C2439: 'std::_Pair_base<_Ty1,_Ty2>::first' : member could not be initialized
          with
          [
              _Ty1=Rocket::Core::TextureHandle ,
              _Ty2=Rocket::Core::Vector2i
          ]
          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\utility(166) : see declaration of 'std::_Pair_base<_Ty1,_Ty2>::first'
          with
          [
              _Ty1=Rocket::Core::TextureHandle ,
              _Ty2=Rocket::Core::Vector2i
          ]
========== Build: 0 succeeded, 1 failed, 0 up-to-date, 0 skipped ==========
