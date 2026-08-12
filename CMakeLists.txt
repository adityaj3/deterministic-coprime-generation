cmake_minimum_required(VERSION 3.22)
project(DeterministicCoprimeEngine VERSION 1.0.0 LANGUAGES CXX)

# Enforce C++20 strictly
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict compiler flags
add_compile_options(-Wall -Wextra -Wpedantic -Werror -O3)

# Locate Boost.Multiprecision and Intel oneTBB (required for C++17/20 parallel algorithms on Linux)
find_package(Boost 1.75 REQUIRED)
find_package(TBB REQUIRED)

# Fetch Catch2 dynamically for unit testing
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# ==============================================================================
# TARGET: Header-Only Engine
# ==============================================================================
add_library(coprime_engine INTERFACE)
target_include_directories(coprime_engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(coprime_engine INTERFACE Boost::headers)

# ==============================================================================
# TARGET: Unit Test Suite
# ==============================================================================
add_executable(coprime_test_suite tests/coprime_test_suite.cpp)
target_link_libraries(coprime_test_suite PRIVATE coprime_engine Catch2::Catch2WithMain TBB::tbb)

# ==============================================================================
# TARGET: Comparative Micro-Benchmark
# ==============================================================================
add_executable(coprime_bench benchmarks/coprime_bench.cpp)
target_link_libraries(coprime_bench PRIVATE coprime_engine)

# ==============================================================================
# TARGET: Coprime Telemetry Inspector
# ==============================================================================
add_executable(coprime_inspector tools/coprime_inspector.cpp)
target_link_libraries(coprime_inspector PRIVATE coprime_engine)

# ==============================================================================
# TARGET: Sanitizer Suite (ASan / UBSan)
# ==============================================================================
add_executable(coprime_sanitizer_suite tests/coprime_test_suite.cpp)
target_compile_options(coprime_sanitizer_suite PRIVATE -fsanitize=address,undefined -g -O1)
target_link_options(coprime_sanitizer_suite PRIVATE -fsanitize=address,undefined)
target_link_libraries(coprime_sanitizer_suite PRIVATE coprime_engine Catch2::Catch2WithMain TBB::tbb)

# Enable CTest integration
enable_testing()
add_test(NAME CoreEngineTests COMMAND coprime_test_suite)
add_test(NAME SanitizerTests COMMAND coprime_sanitizer_suite)
