cmake_minimum_required(VERSION 2.8)

#
# Options
#
option(BUILD_EXAMPLES "Build example applications" ON)
option(GLFW_SYSTEM    "Use the oprating system glfw library" OFF)

find_package(OpenGL REQUIRED)

if(UNIX)
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

if(BUILD_EXAMPLES)

	#
	# ConsoleExample
	#

	add_executable(ConsoleExample ${ConsoleExample_SRCS})
	target_link_libraries(ConsoleExample RuntimeCompiler RuntimeObjectSystem)

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
	endif()

	#
	# glfw
	#
	if(GLFW_SYSTEM)
		set(GLFW_LIBRARIES glfw)
	else()
		include_directories(External/glfw/include)
		add_library( glfw STATIC IMPORTED )
		if(APPLE)
			set_target_properties( glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/glfw/libOSX/libglfw.a )
			set(GLFW_LIBRARIES glfw)
		elseif(WIN32)
			set(GLFW_LIBRARIES glfw)
			if(${CMAKE_CL_64})
				set_target_properties( glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/glfw/libWin/GLFWx64.lib )
			else()
				set_target_properties( glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/glfw/libWin/GLFWx86.lib )
			endif()
		else()
			set_target_properties( glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/glfw/libX11/libglfw.a )
			set(GLFW_LIBRARIES glfw X11)
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
			file(COPY ${CMAKE_SOURCE_DIR}/External/assimp/bin/assimp_release-dll_win32/Assimp64.dll DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
		else()
			set_target_properties( assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/External/assimp/lib/assimp_release-dll_win32/assimp.lib )
			file(COPY ${CMAKE_SOURCE_DIR}/External/assimp/bin/assimp_release-dll_win32/Assimp32.dll DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
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
	
endif()