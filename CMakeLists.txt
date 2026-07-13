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

# Add source to this project's executable.
add_executable (mini-ecs-nau "src/main.cpp"  "src/Entity.h" "src/Component.h" "src/HealthComponent.h" "src/TransformComponent.h" "src/Scene.h" "src/System.h" "src/MovementSystem.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET mini-ecs-nau PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
add_executable(scene_tests tests/test_scene.cpp)
set_property(TARGET scene_tests PROPERTY CXX_STANDARD 20)

add_executable(movement_tests tests/test_movement_system.cpp)
set_property(TARGET movement_tests PROPERTY CXX_STANDARD 20)