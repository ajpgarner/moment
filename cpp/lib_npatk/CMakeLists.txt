include_directories(${NPAToolKit_SOURCE_DIR}/cpp/lib_npatk)

add_library(lib_npatk
        equality_type.cpp
        index_matrix_properties.cpp
        symbol.cpp
        symbol_set.cpp
        symbol_tree.cpp
        symbol_tree_simplify_impl.cpp
        operators/operator.cpp
        operators/operator_collection.cpp
        operators/operator_sequence.cpp
        operators/operator_sequence_generator.cpp operators/multi_operator_iterator.h)