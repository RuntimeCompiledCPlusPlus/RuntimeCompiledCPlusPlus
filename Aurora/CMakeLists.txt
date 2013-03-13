cmake_minimum_required(VERSION 2.8)

#
# Options
#
option(BUILD_EXAMPLES "Build example applications" ON)
option(GLFW_SYSTEM    "Use the oprating system glfw library" OFF)

find_package(OpenGL REQUIRED)

include(cmake/ProjectFiles.cmake)

#
# RuntimeCompiler
#
add_library(RuntimeCompiler SHARED ${RuntimeCompiler_SRCS})

#
# RuntimeObjectSystem
#
add_library(RuntimeObjectSystem SHARED ${RuntimeObjectSystem_SRCS})
target_link_libraries(RuntimeObjectSystem RuntimeCompiler)
if(UNIX)
	target_link_libraries(RuntimeObjectSystem dl)
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
	elseif(WIN32)
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


if(BUILD_EXAMPLES)
	#
	# ConsoleExample
	#
	add_executable(ConsoleExample ${ConsoleExample_SRCS})
	target_link_libraries(ConsoleExample RuntimeCompiler RuntimeObjectSystem)
	#
	# Renderer
	#
	add_library(Renderer SHARED ${Renderer_SRCS})
	target_link_libraries(Renderer ${OPENGL_LIBRARIES})
	#
	# Systems
	#
	add_library(Systems SHARED ${Systems_SRCS})
	#
	# SimpleTest
	#
	add_subdirectory(External/libRocket/Build)
	include_directories(${OPENGL_INCLUDE_DIR} External/libRocket/Include)
	add_executable(SimpleTest ${SimpleTest_SRCS})
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
		)
endif()