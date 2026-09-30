cmake_minimum_required(VERSION 3.22)
project(ParallelRealms VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(SFML)
find_package(Threads REQUIRED)

add_library(realms_core
    src/Level.cpp src/Player.cpp src/HazardSystem.cpp src/Rooms.cpp
    src/Protocol.cpp src/World.cpp src/Network.cpp)
target_include_directories(realms_core PUBLIC include)
target_link_libraries(realms_core PUBLIC SFML::Graphics SFML::Network Threads::Threads)
add_executable(parallel_realms src/main.cpp src/Game.cpp src/NetworkGame.cpp)
target_link_libraries(parallel_realms PRIVATE realms_core SFML::Audio)
add_executable(parallel_realms_server src/server_main.cpp)
target_link_libraries(parallel_realms_server PRIVATE realms_core)
foreach(target realms_core parallel_realms parallel_realms_server)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
# Tests are optional; the game can be distributed without the tests folder.
option(BUILD_TESTING "Build Parallel Realms automated tests" OFF)
include(CTest)
if(BUILD_TESTING)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.cpp")
        add_executable(realms_tests tests/tests.cpp)
        target_link_libraries(realms_tests PRIVATE realms_core)
        add_test(NAME realms_tests COMMAND realms_tests)
    else()
        message(WARNING "Skipping realms_tests: tests/tests.cpp is not present. Game targets remain available.")
    endif()
    find_package(Python3 QUIET COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND AND UNIX AND
       EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/server_smoke.py")
        add_test(NAME server_smoke COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/server_smoke.py
            $<TARGET_FILE:parallel_realms_server>)
    endif()
endif()
