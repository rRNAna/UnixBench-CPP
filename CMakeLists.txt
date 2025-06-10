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
    ${SRCDIR}/arith.cpp
    ${SRCDIR}/big.cpp
    ${SRCDIR}/context1.cpp
    ${SRCDIR}/dummy.cpp
    ${SRCDIR}/execl.cpp
    ${SRCDIR}/fstime.cpp
    ${SRCDIR}/hanoi.cpp
    ${SRCDIR}/pipe.cpp
    ${SRCDIR}/spawn.cpp
    ${SRCDIR}/syscall.cpp
    ${SRCDIR}/looper.cpp
    ${SRCDIR}/timeit.cpp
    ${SRCDIR}/time-polling.cpp
    ${SRCDIR}/dhry.cpp
    ${SRCDIR}/whets.cpp
)

# Output directory
set(PROGDIR ${CMAKE_BINARY_DIR}/pgms)
file(MAKE_DIRECTORY ${PROGDIR})

include_directories(${SRCDIR})

# Each benchmark as separate executable
add_executable(arithoh ${SRCDIR}/arith.cpp)
target_compile_definitions(arithoh PRIVATE arithoh)
set_target_properties(arithoh PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(register ${SRCDIR}/arith.cpp)
target_compile_definitions(register PRIVATE datum="register int")
set_target_properties(register PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(short ${SRCDIR}/arith.cpp)
target_compile_definitions(short PRIVATE datum=short)
set_target_properties(short PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(int ${SRCDIR}/arith.cpp)
target_compile_definitions(int PRIVATE datum=int)
set_target_properties(int PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(long ${SRCDIR}/arith.cpp)
target_compile_definitions(long PRIVATE datum=long)
set_target_properties(long PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(float ${SRCDIR}/arith.cpp)
target_compile_definitions(float PRIVATE datum=float)
set_target_properties(float PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(double ${SRCDIR}/arith.cpp)
target_compile_definitions(double PRIVATE datum=double)
set_target_properties(double PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(hanoi ${SRCDIR}/hanoi.cpp)
set_target_properties(hanoi PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(syscall ${SRCDIR}/syscall.cpp)
set_target_properties(syscall PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(context1 ${SRCDIR}/context1.cpp)
set_target_properties(context1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(pipe ${SRCDIR}/pipe.cpp)
set_target_properties(pipe PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(spawn ${SRCDIR}/spawn.cpp)
set_target_properties(spawn PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(execl ${SRCDIR}/execl.cpp ${SRCDIR}/big.cpp)
set_target_properties(execl PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})
target_compile_options(execl PRIVATE -fno-exceptions -fno-rtti)

add_executable(dhry ${SRCDIR}/dhry.cpp)
set_target_properties(dhry PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(dhry_reg ${SRCDIR}/dhry.cpp)
target_compile_definitions(dhry_reg PRIVATE REG=register)
set_target_properties(dhry_reg PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(looper ${SRCDIR}/looper.cpp)
set_target_properties(looper PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(fstime ${SRCDIR}/fstime.cpp)
set_target_properties(fstime PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

add_executable(whetstone-double ${SRCDIR}/whets.cpp)
target_compile_definitions(whetstone-double PRIVATE DP UNIX UNIXBENCH)
target_link_libraries(whetstone-double m)
set_target_properties(whetstone-double PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})

# Optional: graphical benchmark
# find_package(X11)
# if(X11_FOUND)
#     add_executable(ubgears ${SRCDIR}/ubgears.cpp)
#     target_link_libraries(ubgears ${X11_LIBRARIES} GL)
#     set_target_properties(ubgears PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROGDIR})
# endif()