BOOST LIBRARIES
===============

Current version:
 - 1.49.0

Folders:
 - boost         	- header files
 - stage\lib     	- compiled libraries for windows x86
 - stage\libx64     - compiled libraries for windows x64
 
The paths used are the default paths produces by bjam.
If we later decide to check in the rest of boost and all its libraries, these paths will make that simpler.


Libraries built and currently included:
 - filesystem

Fetching, building, updating Boost:
 - Fetch one of the Boost zips, which are the complete source (including source for the libraries)
 - Drop bjam into the root
 - bjam --help gives details
 - bjam --built-type=minimal --with-filesystem was used to generate the first version
 - copy libraries from stage\lib
 - If new version, update and commit the boost folder also
 
 
For x64 builds, the following works for boost 1.49.0.

Note that you should move the 32bit files you've build somewhere prior to doing this if you have already built them. Could also just have used bjam as above.


Run bootstrap.bat
Open command prompt, go to the dir for boost, and run:
b2 address-model=64