cmake_minimum_required(VERSION 3.22)
project(NPAToolKit)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)
set(USE_CXX_EXCEPTIONS On)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++latest -Wno-unknown-attributes")
ENDIF(MSVC)

add_subdirectory(cpp/lib_npatk)

add_subdirectory(cpp/googletest)
add_subdirectory(cpp/tests)

add_subdirectory(cpp/mex_functions)