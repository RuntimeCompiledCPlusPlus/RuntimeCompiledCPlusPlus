BOOST LIBRARIES
===============

Current version:
 - 1.44.0

Folders:
 - boost         - header files
 - stage\lib     - compiled libraries
 - build_tools   - bjam (for convenience)

The paths used are teh default paths produces by bjam.
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