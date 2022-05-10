add_executable(nptk_tests
        equality_type_tests.cpp
        symbol_set_tests.cpp
        symbol_tree_tests.cpp)

target_link_libraries(nptk_tests
        lib_npatk
        gtest
        gtest_main)