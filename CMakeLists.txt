###########################################################################
# Makefile for UnixBench C++ version
#
# Version: 6.1.0
# Author: rRNA
# Date: 6/10/2025
#
# Description:
# This CMakeLists is used to build the UnixBench C++ version benchmarks.
# It automatically selects clang++ if available, otherwise falls back to g++.
# Static linking is enabled by default to ensure standalone executables.
# Optimizations are adjusted based on platform detection (Linux x86_64, ARM64, or macOS).
#
# Usage:
#   make all        - Build all benchmark binaries
#   make clean      - Remove all generated binaries
#   make spotless   - Remove binaries and temporary results
#
###########################################################################
cmake_minimum_required(VERSION 3.10)
project(unixbench-benchmarks CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/pgms")

# Set C++ standard
find_program(CLANGPP clang++)
find_program(GPP g++)

if(CLANGPP)
    message(STATUS "Using detected compiler: clang++")
    set(CMAKE_CXX_COMPILER ${CLANGPP} CACHE FILEPATH "C++ compiler" FORCE)
elseif(GPP)
    message(STATUS "clang++ not found, using g++")
    set(CMAKE_CXX_COMPILER ${GPP} CACHE FILEPATH "C++ compiler" FORCE)
else()
    message(FATAL_ERROR "No suitable C++ compiler found (clang++ or g++)")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set release build type if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler options
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target arm64-apple-macos -arch arm64 -O3 -fomit-frame-pointer -ffast-math")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -fomit-frame-pointer -ffast-math")
endif()

add_compile_options(${EXTRA_OPTS} -DTIME -Wall -pedantic)

# Source directory
set(SRCDIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB SOURCES
        ${SRCDIR}/*.cpp
)

# Output directory
set(PROGDIR ${CMAKE_BINARY_DIR}/pgms)
file(MAKE_DIRECTORY ${PROGDIR})

include_directories(${SRCDIR})

# Macro for creating executables
function(add_benchmark_executable name source)
    add_executable(${name} ${source})
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})
endfunction()

# Arithmetic variants
foreach(type arithoh register short int long float double)
    set(source ${SRCDIR}/arith.cpp)
    add_executable(${type} ${source})
    if(${type} STREQUAL "arithoh")
        target_compile_definitions(${type} PRIVATE arithoh)
    else()
        target_compile_definitions(${type} PRIVATE datum=${type})
    endif()
    set_target_properties(${type} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})
endforeach()

# Other benchmarks
add_benchmark_executable(hanoi ${SRCDIR}/hanoi.cpp)
add_benchmark_executable(syscall ${SRCDIR}/syscall.cpp)
add_benchmark_executable(context1 ${SRCDIR}/context1.cpp)
add_benchmark_executable(pipe ${SRCDIR}/pipe.cpp)
add_benchmark_executable(spawn ${SRCDIR}/spawn.cpp)
add_executable(execl ${SRCDIR}/execl.cpp ${SRCDIR}/big.cpp)
set_target_properties(execl PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})
target_compile_options(execl PRIVATE -fno-exceptions -fno-rtti)

# Dhrystone
add_executable(dhry ${SRCDIR}/dhry.cpp)
set_target_properties(dhry PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})
add_executable(dhry_reg ${SRCDIR}/dhry.cpp)
target_compile_definitions(dhry_reg PRIVATE REG=register)
set_target_properties(dhry_reg PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_benchmark_executable(looper ${SRCDIR}/looper.cpp)
add_benchmark_executable(fstime ${SRCDIR}/fstime.cpp)

# Whetstone
add_executable(whetstone-double ${SRCDIR}/whets.cpp)
target_compile_definitions(whetstone-double PRIVATE DP UNIX UNIXBENCH)
target_link_libraries(whetstone-double m)
set_target_properties(whetstone-double PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

# Graphics test (optional)
option(ENABLE_GRAPHICS_TESTS "Enable graphics benchmarks" OFF)
if(ENABLE_GRAPHICS_TESTS)
    find_package(X11 REQUIRED)
    add_executable(ubgears ${SRCDIR}/ubgears.cpp)
    target_link_libraries(ubgears ${X11_LIBRARIES} GL Xext)
    include_directories(${X11_INCLUDE_DIR})
    set_target_properties(ubgears PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})
endif()