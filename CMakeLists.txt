cmake_minimum_required(VERSION 3.22)

project(ParallelRealms VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SFML)
find_package(Threads REQUIRED)

add_executable(parallel_realms
    src/main.cpp
    src/Game.cpp
    src/Level.cpp
    src/Player.cpp
    src/HazardSystem.cpp
)

target_include_directories(parallel_realms PRIVATE include)
target_link_libraries(parallel_realms PRIVATE SFML::Graphics Threads::Threads)

if(MSVC)
    target_compile_options(parallel_realms PRIVATE /W4)
else()
    target_compile_options(parallel_realms PRIVATE -Wall -Wextra -Wpedantic)
endif()

