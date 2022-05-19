cmake_minimum_required(VERSION 3.22)
project(NPAToolKit)

set(CMAKE_CXX_STANDARD 20)
set(USE_CXX_EXCEPTIONS On)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++latest -Wno-unknown-attributes")
ENDIF(MSVC)

include_directories(lib)

add_library(lib_npatk
        lib/symbol.cpp
        lib/equality_type.cpp
        lib/symbol_set.cpp
        lib/symbol_tree.cpp
        lib/symbol_tree_simplify_impl.cpp
        lib/index_matrix_properties.cpp)

add_subdirectory(googletest)
add_subdirectory(tests)

add_subdirectory(mex_functions)