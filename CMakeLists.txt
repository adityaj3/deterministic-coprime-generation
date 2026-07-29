cmake_minimum_required(VERSION 3.20)
project(coprime_engine LANGUAGES CXX)

# Enforce C++20 strictly
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find installed dependencies
find_package(Boost REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(TBB REQUIRED)

# Common include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# -----------------------------------------------------------------------------
# Target 1: Simple Verification Executable (main.cpp)
# -----------------------------------------------------------------------------
add_executable(coprime_test main.cpp)
target_link_libraries(coprime_test PRIVATE Boost::boost)

# -----------------------------------------------------------------------------
# Target 2: Exhaustive Catch2 Test Suite (coprime_test_suite.cpp)
# -----------------------------------------------------------------------------
add_executable(coprime_test_suite coprime_test_suite.cpp)
target_link_libraries(coprime_test_suite PRIVATE
        Boost::boost
        Catch2::Catch2WithMain
        TBB::tbb
)

# Compiler Warning Flags
if(MSVC)
    target_compile_options(coprime_test PRIVATE /W4)
    target_compile_options(coprime_test_suite PRIVATE /W4)
else()
    target_compile_options(coprime_test PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(coprime_test_suite PRIVATE -Wall -Wextra)
endif()