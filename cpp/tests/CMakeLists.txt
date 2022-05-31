include_directories(${NPAToolKit_SOURCE_DIR}/cpp/lib_npatk)

add_executable(npatk_tests
        symbol_tests.cpp
        symbol_set_tests.cpp
        equality_type_tests.cpp
        symbol_node_and_link_tests.cpp
        symbol_tree_tests.cpp
        )

target_link_libraries(npatk_tests
        lib_npatk
        gtest
        gtest_main)