include_directories(${NPAToolKit_SOURCE_DIR}/cpp/lib_npatk)

add_library(lib_npatk
        operators/context.cpp
        operators/moment_matrix.cpp
        operators/operator.cpp
        operators/operator_sequence.cpp
        operators/operator_sequence_generator.cpp
        operators/party_info.cpp
        symbolic/equality_type.cpp
        symbolic/index_matrix_properties.cpp
        symbolic/symbol.cpp
        symbolic/symbol_expression.cpp
        symbolic/symbol_set.cpp
        symbolic/symbol_tree.cpp
        symbolic/symbol_tree_simplify_impl.cpp
    )