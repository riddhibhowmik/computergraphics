cmake_minimum_required(VERSION 3.15)
project(ParticleForge CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME} 
src/main.cpp
src/Project.cpp
src/SnowEffect.cpp
src/UIManager.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)