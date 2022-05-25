cmake_minimum_required(VERSION 3.22)
project(NPAToolKit)

set(CMAKE_CXX_STANDARD 20)
set(USE_CXX_EXCEPTIONS On)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++latest -Wno-unknown-attributes")
ENDIF(MSVC)

add_subdirectory(lib)

add_subdirectory(googletest)
add_subdirectory(tests)

add_subdirectory(mex_functions)