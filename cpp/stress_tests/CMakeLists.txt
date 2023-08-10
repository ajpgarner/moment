include_directories(${Moment_SOURCE_DIR}/cpp/lib_moment)

# Add BFF stress test
add_executable(stresstest_bff
    test_bff.cpp
)
target_link_libraries(stresstest_bff
    lib_moment)