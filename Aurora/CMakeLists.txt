cmake_minimum_required(VERSION 2.8)

add_subdirectory(External/libRocket/Build)

include_directories(
	External/libRocket/Include
)

set(RCC_SOURCE
	Renderer/AURenderContext.cpp
	Renderer/AURenMesh.cpp
	RuntimeCompiler/BuildTool.cpp
	RuntimeCompiler/Compiler_PlatformPosix.cpp
	RuntimeCompiler/FileChangeNotifier.cpp
	RuntimeCompiler/FileMonitor.cpp
	RuntimeCompiler/SimpleFileWatcher/FileWatcher.cpp
	RuntimeCompiler/SimpleFileWatcher/FileWatcherLinux.cpp
	RuntimeObjectSystem/ObjectFactorySystem/ObjectFactorySystem.cpp
	RuntimeObjectSystem/ObjectInterfacePerModuleSource.cpp
	RuntimeObjectSystem/RuntimeObjectSystem.cpp
	RuntimeObjectSystem/RuntimeObjectSystem_PlatformPosix.cpp
	RuntimeObjectSystem/SimpleSerializer/SimpleSerializer.cpp
)


set(CONSOLE_EXAMPLE_SOURCE
	Examples/ConsoleExample/ConsoleExample.cpp
	Examples/ConsoleExample/ConsoleGame.cpp
	Examples/ConsoleExample/RuntimeObject01.cpp
	Examples/ConsoleExample/StdioLogSystem.cpp
)

set(SIMPLE_TEST_SOURCE
	Examples/SimpleTest/Behavior_Infected_Approach.cpp
	Examples/SimpleTest/Behavior_Infected_Combat.cpp
	Examples/SimpleTest/Behavior_Infected_HuntRBC.cpp
	Examples/SimpleTest/Behavior_Infected_HuntWBC.cpp
	Examples/SimpleTest/Behavior_Infected_Idle.cpp
	Examples/SimpleTest/Behavior_RBC_Approach.cpp
	Examples/SimpleTest/Behavior_RBC_Combat.cpp
	Examples/SimpleTest/Behavior_RBC_Evade.cpp
	Examples/SimpleTest/Behavior_RBC_Idle.cpp
	Examples/SimpleTest/BehaviorTree_Infected.cpp
	Examples/SimpleTest/BehaviorTreeManager.cpp
	Examples/SimpleTest/BehaviorTree_RBC.cpp
	Examples/SimpleTest/BehaviorTree_Virus.cpp
	Examples/SimpleTest/BehaviorTree_WBC.cpp
	Examples/SimpleTest/Behavior_Virus_Approach.cpp
	Examples/SimpleTest/Behavior_Virus_Combat.cpp
	Examples/SimpleTest/Behavior_Virus_HuntRBC.cpp
	Examples/SimpleTest/Behavior_Virus_HuntWBC.cpp
	Examples/SimpleTest/Behavior_Virus_Idle.cpp
	Examples/SimpleTest/Behavior_WBC_Approach.cpp
	Examples/SimpleTest/Behavior_WBC_Combat.cpp
	Examples/SimpleTest/Behavior_WBC_Idle.cpp
	Examples/SimpleTest/Behavior_WBC_Patrol.cpp
	Examples/SimpleTest/BlackboardManager.cpp
	Examples/SimpleTest/CameraControl.cpp
	Examples/SimpleTest/CompilerLogger.cpp
	Examples/SimpleTest/CompilingNotification.cpp
	Examples/SimpleTest/Console.cpp
	Examples/SimpleTest/Environment.cpp
	Examples/SimpleTest/FPSCounter.cpp
	Examples/SimpleTest/Game.cpp
	Examples/SimpleTest/GameManager.cpp
	Examples/SimpleTest/GameObject.cpp
	Examples/SimpleTest/InputManager.cpp
	Examples/SimpleTest/LightingControl.cpp
	Examples/SimpleTest/Main.cpp
	Examples/SimpleTest/MainMenu.cpp
	Examples/SimpleTest/MainObject.cpp
	Examples/SimpleTest/PerceptionManager.cpp
	Examples/SimpleTest/PhysicsManager.cpp
	Examples/SimpleTest/SplashScreen.cpp
	Systems/AssetSystem/AssetSystem.cpp
	Systems/EntitySystem/EntitySystem.cpp
	Systems/GUISystem/GUIDocument.cpp
	Systems/GUISystem/GUIElement.cpp
	Systems/GUISystem/GUISystem.cpp
	Systems/GUISystem/ReferenceCountable.cpp
	Systems/LogSystem/FileLogSystem/FileLogSystem.cpp
	Systems/LogSystem/MultiLogSystem/MultiLogSystem.cpp
	Systems/LogSystem/RocketLogSystem/RocketLogSystem.cpp
	Systems/LogSystem/ThreadsafeLogSystem/ThreadsafeLogSystem_PlatformWindows.cpp
	Systems/RocketLibSystem/Input.cpp
	Systems/RocketLibSystem/InputGLFW.cpp
	Systems/RocketLibSystem/RocketLibSystem.cpp
	Systems/RocketLibSystem/RocketLibSystemFileInterface.cpp
	Systems/RocketLibSystem/RocketLibSystemGLFW.cpp
	Systems/RocketLibSystem/RocketLibSystemRenderInterfaceOpenGL.cpp
	Systems/RocketLibSystem/RocketLibSystemSystemInterface.cpp
	Systems/Systems.cpp
	Systems/TimeSystem/TimeSystem.cpp
	Renderer/AURenderContext.cpp
	Renderer/AURenMesh.cpp
)

add_library(RCC SHARED ${RCC_SOURCE} )

add_executable(ConsoleExample ${CONSOLE_EXAMPLE_SOURCE})
target_link_libraries(ConsoleExample RCC RocketCore RocketControls RocketDebugger GL glfw)

add_executable(SimpleTest ${SIMPLE_TEST_SOURCE})
target_link_libraries(SimpleTest RCC RocketCore RocketControls RocketDebugger GL glfw)
