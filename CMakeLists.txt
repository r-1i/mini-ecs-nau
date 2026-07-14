	# CMakeList.txt : CMake project for mini-ecs-nau, include source and define
	# project specific logic here.
	#
	cmake_minimum_required (VERSION 3.16)

	# Detect sccache before project() - CMAKE_CXX_COMPILER_LAUNCHER is
	# only read once, when project() first configures the compiler.
	# Falls back to a normal (uncached) build if sccache isn't
	# installed - not an error.
	find_program(SCCACHE_PROGRAM sccache)
	if(SCCACHE_PROGRAM)
	  message(STATUS "sccache found: ${SCCACHE_PROGRAM}")
	  set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
	else()
	  message(STATUS "sccache not found, building without a compiler cache")
	endif()

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

	# Headers here are for IDE grouping only (Visual Studio's CMake
	# generator needs them listed to show up in Solution Explorer) -
	# compilation only actually depends on the .cpp sources below.
	set(ECS_HEADERS
	  "src/core/entity.h"
	  "src/core/component.h"
	  "src/core/i_savable.h"
	  "src/core/i_component_pool.h"
	  "src/core/component_pool.h"
	  "src/core/type_id.h"
	  "src/core/component_registry.h"
	  "src/core/scene.h"
	  "src/core/system.h"
	  "src/components/transform_component.h"
	  "src/components/health_component.h"
	  "src/systems/movement_system.h"
	  "src/profiling/profiler.h"
	)

	# Non-template logic lives in .cpp - only ComponentPool<T> and Scene's
	# template methods stay header-only, since templates must be visible
	# at the instantiation point. Every executable below links the same
	# set: small project, not worth tracking per-target subsets.
	set(ECS_SOURCES
	  "src/core/component_registry.cpp"
	  "src/core/scene.cpp"
	  "src/components/transform_component.cpp"
	  "src/components/health_component.cpp"
	  "src/systems/movement_system.cpp"
	  "src/profiling/profiler.cpp"
	)

	# Add source to this project's executable.
	add_executable (mini-ecs-nau "src/main.cpp" ${ECS_SOURCES} ${ECS_HEADERS})

	add_executable(scene_tests tests/test_scene.cpp ${ECS_SOURCES} ${ECS_HEADERS})
	add_executable(movement_tests tests/test_movement_system.cpp ${ECS_SOURCES} ${ECS_HEADERS})
	add_executable(memory_tests tests/test_memory.cpp ${ECS_SOURCES} ${ECS_HEADERS})
	add_executable(serialization_tests tests/test_serialization.cpp ${ECS_SOURCES} ${ECS_HEADERS})
	# Not a correctness test - benchmark harness for issue 10 (profiler).
	# Run in Release config; Debug numbers aren't representative.
	add_executable(scene_benchmark tests/benchmark_scene.cpp ${ECS_SOURCES} ${ECS_HEADERS})

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

target_precompile_headers(mini-ecs-nau PRIVATE <nlohmann/json.hpp>)
target_precompile_headers(scene_tests REUSE_FROM mini-ecs-nau)
target_precompile_headers(movement_tests REUSE_FROM mini-ecs-nau)
target_precompile_headers(memory_tests REUSE_FROM mini-ecs-nau)
target_precompile_headers(serialization_tests REUSE_FROM mini-ecs-nau)
target_precompile_headers(scene_benchmark REUSE_FROM mini-ecs-nau)
