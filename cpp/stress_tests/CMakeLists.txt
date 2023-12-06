include_directories(${Moment_SOURCE_DIR}/cpp/lib_moment)

# Add BFF stress test
add_executable(stresstest_bff
    test_bff.cpp
)
target_link_libraries(stresstest_bff
    lib_moment)

# Add inflation stress test
add_executable(stresstest_inflation
    test_inflated_triangle.cpp
)
target_link_libraries(stresstest_inflation
    lib_moment)

# Add symmetrization stress test
add_executable(stresstest_symmetrization
    test_symmetrized_i4422.cpp
)
target_link_libraries(stresstest_symmetrization
    lib_moment)

# Add Pauli lattice stress test
add_executable(stresstest_pauli_lattice
    test_pauli_lattice.cpp
)
target_link_libraries(stresstest_pauli_lattice
    lib_moment)