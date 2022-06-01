include_directories(${NPAToolKit_SOURCE_DIR}/cpp/lib_npatk)

add_library(lib_npatk
        equality_type.cpp
        index_matrix_properties.cpp
        operator_sequence.cpp
        symbol.cpp
        symbol_set.cpp
        symbol_tree.cpp
        symbol_tree_simplify_impl.cpp
    )