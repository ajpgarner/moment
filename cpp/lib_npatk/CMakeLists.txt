include_directories(${NPAToolKit_SOURCE_DIR}/cpp/lib_npatk)

add_library(lib_npatk
        symbol.cpp
        equality_type.cpp
        symbol_set.cpp
        symbol_tree.cpp
        symbol_tree_simplify_impl.cpp
        index_matrix_properties.cpp)