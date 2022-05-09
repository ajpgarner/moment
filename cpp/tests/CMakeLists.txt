add_executable(nptk_tests
        symbol_set_tests.cpp
        symbol_tree_tests.cpp)

target_link_libraries(nptk_tests
        lib_npatk
        gtest
        gtest_main)