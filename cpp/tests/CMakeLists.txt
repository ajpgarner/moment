include_directories(${Moment_SOURCE_DIR}/cpp/lib_moment)

include(GoogleTest)

add_executable(moment_tests
        matrix/matrix_tests.cpp
        matrix/moment_matrix_tests.cpp
        matrix/polynomial_localizing_matrix_tests.cpp
        matrix/polynomial_matrix_tests.cpp
        multithreading/extended_matrix_tests.cpp
        multithreading/localizing_matrix_tests.cpp
        multithreading/moment_matrix_tests.cpp
        multithreading/substituted_matrix_tests.cpp
        multithreading/queue_tests.cpp
        operators/hashed_sequence_tests.cpp
        operators/localizing_matrix_tests.cpp
        operators/multi_operator_iterator_tests.cpp
        operators/operator_sequence_tests.cpp
        operators/operator_sequence_generator_tests.cpp
        scenarios/context_tests.cpp
        scenarios/contextual_os_tests.cpp
        scenarios/dictionary_map_tests.cpp
        scenarios/probability_tensor_test_helpers.cpp
        scenarios/algebraic/algebraic_context_tests.cpp
        scenarios/algebraic/algebraic_precontext_tests.cpp
        scenarios/algebraic/monomial_substitution_rule_tests.cpp
        scenarios/algebraic/name_table_tests.cpp
        scenarios/algebraic/rule_book_tests.cpp
        scenarios/derived/lu_mcp_tests.cpp
        scenarios/derived/map_core_tests.cpp
        scenarios/derived/symbol_table_map_tests.cpp
        scenarios/imported/imported_matrix_system_tests.cpp
        scenarios/inflation/causal_network_tests.cpp
        scenarios/inflation/extended_matrix_tests.cpp
        scenarios/inflation/extension_suggester_tests.cpp
        scenarios/inflation/factor_table_tests.cpp
        scenarios/inflation/inflation_collins_gisin_tests.cpp
        scenarios/inflation/inflation_context_tests.cpp
        scenarios/inflation/inflation_matrix_system_tests.cpp
        scenarios/inflation/inflation_probability_tensor_tests.cpp
        scenarios/locality/locality_collins_gisin_tests.cpp
        scenarios/locality/locality_context_tests.cpp
        scenarios/locality/locality_probability_tensor_tests.cpp
        scenarios/locality/locality_osg_tests.cpp
        scenarios/locality/party_tests.cpp
        scenarios/locality/virtual_polynomial_view_tests.cpp
        scenarios/symmetry/group_tests.cpp
        scenarios/symmetry/representation_mapper_tests.cpp
        scenarios/symmetry/symmetrized_matrix_system_tests.cpp
        symbolic/full_combo_ordering_tests.cpp
        symbolic/monomial_tests.cpp
        symbolic/order_symbols_by_hash_tests.cpp
        symbolic/polynomial_tests.cpp
        symbolic/polynomial_real_imag_tests.cpp
        symbolic/polynomial_to_basis_tests.cpp
        symbolic/polynomial_to_basis_mask_tests.cpp
        symbolic/polynomial_index_storage_tests.cpp
        symbolic/symbol_table_tests.cpp
        symbolic/symbolic_matrix_helpers.cpp
        symbolic/rules/moment_rule_tests.cpp
        symbolic/rules/moment_rule_split_tests.cpp
        symbolic/rules/moment_rulebook_tests.cpp
        symbolic/rules/moment_rulebook_factor_tests.cpp
        symbolic/rules/moment_rulebook_to_basis_tests.cpp
        tensor/index_flattener_tests.cpp
        tensor/multi_dimensional_index_iterator_tests.cpp
        tensor/tensor_tests.cpp
        tensor/multi_dimensional_offset_index_iterator_tests.cpp
        tensor/square_matrix_tests.cpp
        tensor/auto_storage_tensor_tests.cpp
        utilities/alphabetic_namer_tests.cpp
        utilities/combination_tests.cpp
        utilities/dynamic_bitset_tests.cpp
        utilities/first_intersection_tests.cpp
        utilities/float_utils_tests.cpp
        utilities/index_tree_tests.cpp
        utilities/ipow_tests.cpp
        utilities/iter_tuple_tests.cpp
        utilities/kronecker_power_tests.cpp
        utilities/linear_map_merge_tests.cpp
        utilities/multi_partition_tests.cpp
        utilities/ordered_permutation_tests.cpp
        utilities/persistent_storage_tests.cpp
        utilities/recursive_storage_tests.cpp
        utilities/shortlex_hasher_tests.cpp
        utilities/small_vector_tests.cpp
        utilities/triangular_index_iterator_tests.cpp
        utilities/utf_conversion_tests.cpp
        utilities/variable_chunk_range_tests.cpp
)

target_link_libraries(moment_tests PRIVATE
    lib_moment
    gtest
    gtest_main
)

if(CMAKE_BUILD_TYPE STREQUAL "ASan")
    target_link_directories(moment_tests PRIVATE "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/10.0.0/lib/windows")
    target_link_libraries(moment_tests PRIVATE  clang_rt.asan_dynamic-x86_64 clang_rt.asan_dynamic_runtime_thunk-x86_64)
    target_link_options(moment_tests PRIVATE "/wholearchive:clang_rt.asan_dynamic_runtime_thunk-x86_64.lib")
    target_link_options(moment_tests PUBLIC "/DEBUG")
else()
    gtest_discover_tests(moment_tests)
endif()

