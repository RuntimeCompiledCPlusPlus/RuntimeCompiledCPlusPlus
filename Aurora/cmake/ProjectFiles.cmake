
#
# RuntimeCompiler Source
#

aux_source_directory(RuntimeCompiler RuntimeCompiler_SRCS)
aux_source_directory(RuntimeCompiler/SimpleFileWatcher SimpleFileWatcher_SRCS)

if(UNIX)
	list(REMOVE_ITEM RuntimeCompiler_SRCS "RuntimeCompiler/Compiler_PlatformWindows.cpp")
	list(REMOVE_ITEM SimpleFileWatcher_SRCS "RuntimeCompiler/SimpleFileWatcher/FileWatcherWin32.cpp")
	if(APPLE)
		list(REMOVE_ITEM SimpleFileWatcher_SRCS "RuntimeCompiler/SimpleFileWatcher/FileWatcherLinux.cpp")
	else()
		list(REMOVE_ITEM SimpleFileWatcher_SRCS "RuntimeCompiler/SimpleFileWatcher/FileWatcherOSX.cpp")
	endif()
else()
	list(REMOVE_ITEM RuntimeCompiler_SRCS "RuntimeCompiler/Compiler_PlatformPosix.cpp")
	list(REMOVE_ITEM SimpleFileWatcher_SRCS "RuntimeCompiler/SimpleFileWatcher/FileWatcherOSX.cpp")
	list(REMOVE_ITEM SimpleFileWatcher_SRCS "RuntimeCompiler/SimpleFileWatcher/FileWatcherLinux.cpp")
endif()

set(RuntimeCompiler_SRCS ${RuntimeCompiler_SRCS} ${SimpleFileWatcher_SRCS})

#
# RuntimeObjectSystem Source
#

aux_source_directory(RuntimeObjectSystem RuntimeObjectSystem_SRCS)
aux_source_directory(RuntimeObjectSystem/ObjectFactorySystem ObjectFactorySystem_SRCS)
aux_source_directory(RuntimeObjectSystem/SimpleSerializer SimpleSerializer_SRCS)

set(RuntimeCompiler_SRCS ${RuntimeCompiler_SRCS} ${ObjectFactorySystem_SRCS} ${SimpleSerializer_SRCS})

if(UNIX)
	list(REMOVE_ITEM RuntimeObjectSystem_SRCS "RuntimeObjectSystem/RuntimeObjectSystem_PlatformWindows.cpp")
else()
	list(REMOVE_ITEM RuntimeObjectSystem_SRCS "RuntimeObjectSystem/RuntimeObjectSystem_PlatformPosix.cpp")
endif()

#
# Example applications
#

if(BUILD_EXAMPLES)
	#
	# ConsoleExample Source
	#
	aux_source_directory(Examples/ConsoleExample ConsoleExample_SRCS)
	#
	# SimpleTest Source
	#
	aux_source_directory(Examples/SimpleTest SimpleTest_SRCS)
	#
	# Renderer Source
	#
	aux_source_directory(Renderer Renderer_SRCS)
	#
	# Systems Source
	#
	#aux_source_directory(Systems Systems_SRCS)
	file(GLOB_RECURSE Systems_SRCS "Systems/*.cpp")
endif()

