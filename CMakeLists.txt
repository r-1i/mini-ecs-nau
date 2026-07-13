	# CMakeList.txt : CMake project for mini-ecs-nau, include source and define
	# project specific logic here.
	#
	cmake_minimum_required (VERSION 3.10)

	# Enable Hot Reload for MSVC compilers if supported.
	if (POLICY CMP0141)
	  cmake_policy(SET CMP0141 NEW)
	  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
	endif()

	project ("mini-ecs-nau")

	# Set globally (not per-target) so every target defined below -
	# the main executable and all test executables - gets the same
	# standard. Structured bindings and fold expressions used
	# throughout src/ require at least C++17.
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	# Add source to this project's executable.
	add_executable (mini-ecs-nau "src/main.cpp" "src/Entity.h" "src/Component.h" "src/HealthComponent.h" "src/TransformComponent.h" "src/Scene.h" "src/System.h" "src/MovementSystem.h" "src/ISavable.h" "src/ComponentRegistry.h" "src/Profiler.h" "src/IComponentPool.h" "src/ComponentPool.h")

	add_executable(scene_tests tests/test_scene.cpp "src/ComponentRegistry.h" "src/IComponentPool.h" "src/ComponentPool.h")
	add_executable(movement_tests tests/test_movement_system.cpp "src/ComponentRegistry.h" "src/IComponentPool.h" "src/ComponentPool.h")
	add_executable(memory_tests tests/test_memory.cpp "src/ComponentRegistry.h" "src/IComponentPool.h" "src/ComponentPool.h")
	add_executable(serialization_tests tests/test_serialization.cpp "src/ComponentRegistry.h" "src/IComponentPool.h" "src/ComponentPool.h")
	# Not a correctness test - benchmark harness for issue 10 (profiler).
	# Run in Release config; Debug numbers aren't representative.
	add_executable(scene_benchmark tests/benchmark_scene.cpp "src/ComponentRegistry.h" "src/Profiler.h" "src/IComponentPool.h" "src/ComponentPool.h")

	include(FetchContent)
	FetchContent_Declare(
	  json
	  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
	)
	FetchContent_MakeAvailable(json)
	target_link_libraries(mini-ecs-nau PRIVATE nlohmann_json::nlohmann_json)
	target_link_libraries(scene_tests PRIVATE nlohmann_json::nlohmann_json)
	target_link_libraries(movement_tests PRIVATE nlohmann_json::nlohmann_json)
	target_link_libraries(memory_tests PRIVATE nlohmann_json::nlohmann_json)
	target_link_libraries(serialization_tests PRIVATE nlohmann_json::nlohmann_json)
	target_link_libraries(scene_benchmark PRIVATE nlohmann_json::nlohmann_json)