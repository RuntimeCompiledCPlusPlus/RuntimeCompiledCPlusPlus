cmake_minimum_required(VERSION 2.8)

project( RuntimeCompiledCPlusPlus )
#
# Options
#
option(BUILD_EXAMPLES "Build example applications" ON)
option(GLFW_SYSTEM    "Use the operating system glfw library" OFF)

if(UNIX AND NOT APPLE)
	set(BUILD_TYPE SHARED)
else()
	set(BUILD_TYPE STATIC)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

include(cmake/ProjectFiles.cmake)

#
# RuntimeCompiler
#
add_library(RuntimeCompiler ${BUILD_TYPE} ${RuntimeCompiler_SRCS})

#
# RuntimeObjectSystem
#
add_library(RuntimeObjectSystem ${BUILD_TYPE} ${RuntimeObjectSystem_SRCS})
target_link_libraries(RuntimeObjectSystem RuntimeCompiler)
if(UNIX)
	target_link_libraries(RuntimeObjectSystem dl)
endif()

#
# Make Install
#
install(DIRECTORY RuntimeObjectSystem/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/RuntimeObjectSystem
	FILES_MATCHING PATTERN "*.h")
install(DIRECTORY RuntimeCompiler/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/RuntimeCompiler
	FILES_MATCHING PATTERN "*.h")
install(TARGETS RuntimeObjectSystem RuntimeCompiler 
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

if(BUILD_EXAMPLES)


	#
	# ConsoleExample
	#

	add_executable(ConsoleExample ${ConsoleExample_SRCS})
	target_link_libraries(ConsoleExample RuntimeCompiler RuntimeObjectSystem)

	
	find_package(OpenGL)
	if(OpenGL_FOUND)
		#
		# Renderer
		#
	
		add_library(Renderer ${BUILD_TYPE} ${Renderer_SRCS})
		target_link_libraries(Renderer ${OPENGL_LIBRARIES} ${ASSIMP_LIBRARIES})
	
		#
		# Systems
		#
	
		add_library(Systems ${BUILD_TYPE} ${Systems_SRCS})
		
		#
		# Freetype WIN32
		#
	
		if(WIN32)
			set(FREETYPE_INCLUDE_DIR_freetype2 ${CMAKE_SOURCE_DIR}/External/support/freetype/include/freetype)
			set(FREETYPE_INCLUDE_DIR_ft2build ${CMAKE_SOURCE_DIR}/External/support/freetype/include)
			if(${CMAKE_CL_64})
				set(FREETYPE_LIBRARY ${CMAKE_SOURCE_DIR}/External/support/lib/freetype2410MTx64.lib)
			else()
				set(FREETYPE_LIBRARY ${CMAKE_SOURCE_DIR}/External/support/lib/freetype2410MT.lib)
			endif()
		elseif(APPLE)
			set(FREETYPE_INCLUDE_DIR_freetype2 ${CMAKE_SOURCE_DIR}/External/support/freetype/include/freetype)
			set(FREETYPE_INCLUDE_DIR_ft2build ${CMAKE_SOURCE_DIR}/External/support/freetype/include)
			FIND_LIBRARY(ZLIB_LIBRARY libs)
			MARK_AS_ADVANCED(ZLIB_LIBRARY)
			set(FREETYPE_LIBRARY ${CMAKE_SOURCE_DIR}/External/support/lib/MacOSX/libfreetype.a ${ZLIB_LIBRARY})
		endif()
	
		#
		# glfw
		#
		if(GLFW_SYSTEM)
			set(GLFW_LIBRARIES glfw)
		else()
			include_directories(External/glfw/include)
			if(WIN32)
					set(GLFW_LIBRARIES glfw winmm)
					add_subdirectory( ${CMAKE_SOURCE_DIR}/External/glfw/projects )
			else()
				add_library( glfw STATIC IMPORTED )
				if(APPLE)
					FIND_LIBRARY(COCOA_LIBRARY Cocoa)
					FIND_LIBRARY(IOKIT_LIBRARY IOKit)
					MARK_AS_ADVANCED(COCOA_LIBRARY IOKIT_LIBRARY)
					set_target_properties( glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/glfw/libOSX/libglfw.a )
					set(GLFW_LIBRARIES glfw ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
				else()
					set_target_properties( glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/glfw/libX11/libglfw.a )
					set(GLFW_LIBRARIES glfw X11 pthread)
				endif()
			endif()
		endif()
	
		#
		# assimp
		#
	
		if(WIN32)
			add_library( assimp STATIC IMPORTED )
			set(ASSIMP_LIBRARIES assimp)
			if(${CMAKE_CL_64})
				set_target_properties( assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/assimp/lib/assimp_release-dll_x64/assimp.lib )
				set(ASSIMP_DLL ${CMAKE_SOURCE_DIR}/External/assimp/bin/assimp_release-dll_win32/Assimp64.dll)
			else()
				set_target_properties( assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/assimp/lib/assimp_release-dll_win32/assimp.lib )
				set(ASSIMP_DLL ${CMAKE_SOURCE_DIR}/External/assimp/bin/assimp_release-dll_win32/Assimp32.dll)
			endif()
			if(CMAKE_GENERATOR STREQUAL "NMake Makefiles")
				file(COPY ${ASSIMP_DLL} DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
			else()
				foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
					file(COPY ${ASSIMP_DLL} DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OUTPUTCONFIG})
				endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
			endif()
		endif()
	
		#
		# SimpleTest
		#
	
		add_subdirectory(External/libRocket/Build)
		include_directories(
			${OPENGL_INCLUDE_DIR} 
			External/libRocket/Include
			External/assimp/include
		)
		if(WIN32)
			add_executable(SimpleTest WIN32 ${SimpleTest_SRCS})
		else()
			add_executable(SimpleTest ${SimpleTest_SRCS})
		endif()
		target_link_libraries(SimpleTest 
				RuntimeCompiler 
				RuntimeObjectSystem 
				Renderer
				Systems
				RocketCore 
				RocketControls 
				RocketDebugger
				${OPENGL_LIBRARIES} 
				${GLFW_LIBRARIES}
				${ASSIMP_LIBRARIES}
			)
		if(MSVC)
			set_target_properties(SimpleTest ConsoleExample PROPERTIES COMPILE_FLAGS "/FC")
		else()
			Set(CMAKE_CXX_FLAGS  "-DCOMPILE_PATH=\"\\\"$(PWD)\\\"\"")
		endif()
	else()  # OpenGL_FOUND
		message(WARNING "OpenGL not found, not creating graphical example")
	endif() # OpenGL_FOUND
endif()
