cmake_minimum_required(VERSION 3.22)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)
        if (NOT exists "/usr/bin/g++-12")
            message(FATAL_ERROR "This library can only be compiled with GCC version 12.0 or newer.")
        else()
            set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
        endif()
    endif()
endif()

project(NPAToolKit)

set(USE_CXX_EXCEPTIONS On)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++latest -Wno-unknown-attributes")
endif(MSVC)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -static-libstdc++")
endif()


add_subdirectory(cpp/lib_npatk)

add_subdirectory(cpp/googletest)
add_subdirectory(cpp/tests)

add_subdirectory(cpp/mex_functions)
