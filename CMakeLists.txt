cmake_minimum_required(VERSION 3.22)
if(UNIX)
    set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ASan")

project(Moment)


set(USE_CXX_EXCEPTIONS On)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /std:c++20")
endif(MSVC)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -static-libstdc++")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -static")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(MOMENT_DEBUG)
    if (UNIX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "ASan")
    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MD /Z7 /Od")
        add_compile_definitions(_DISABLE_STRING_ANNOTATION=1 _DISABLE_VECTOR_ANNOTATION=1)

    endif()
    add_compile_options("-fsanitize=address")
    add_compile_definitions(MOMENT_DEBUG)
    add_compile_definitions(MOMENT_ASAN)
    add_link_options("-fsanitize=address")
endif()

include_directories(cpp/eigen)

add_subdirectory(cpp/lib_moment)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cpp/googletest/CMakeLists.txt")
    enable_testing()
    add_subdirectory(cpp/googletest)
    add_subdirectory(cpp/tests)
endif()

add_subdirectory(cpp/mex_functions)

