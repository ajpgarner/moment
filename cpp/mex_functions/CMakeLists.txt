set(MATLAB_FIND_DEBUG)

find_package(Matlab)

include_directories(${NPAToolKit_SOURCE_DIR}/cpp/lib_npatk)
include_directories(${NPAToolKit_SOURCE_DIR}/cpp/mex_functions)

matlab_add_mex(NAME npatk_mex
        SRC
            mex_entry.cpp
            mex_main.cpp
            storage_manager.cpp
            fragments/enumerate_symbols.cpp
            fragments/export_implicit_symbols.cpp
            fragments/export_substitution_list.cpp
            fragments/export_monomial_rules.cpp
            fragments/export_operator_matrix.cpp
            fragments/export_symbol_tree_properties.cpp
            fragments/export_symbol_table.cpp
            fragments/identify_nonhermitian_elements.cpp
            fragments/identify_nonsymmetric_elements.cpp
            fragments/read_monomial_rules.cpp
            fragments/read_symbol_or_fail.cpp
            fragments/substitute_elements_using_tree.cpp
            utilities/io_parameters.cpp
            utilities/make_sparse_matrix.cpp
            utilities/read_as_scalar.cpp
            utilities/read_as_string.cpp
            utilities/read_as_vector.cpp
            utilities/reflection.cpp
            utilities/reporting.cpp
            matlab_classes/matlab_class.cpp
            functions/alphabetic_name.cpp
            functions/collins_gisin.cpp
            functions/complete.cpp
            functions/function_list.cpp
            functions/generate_basis.cpp
            functions/localizing_matrix.cpp
            functions/make_hermitian.cpp
            functions/make_symmetric.cpp
            functions/moment_matrix.cpp
            functions/new_algebraic_matrix_system.cpp
            functions/new_locality_matrix_system.cpp
            functions/operator_matrix.cpp
            functions/probability_table.cpp
            functions/release.cpp
            functions/rules.cpp
            functions/symbol_table.cpp
            functions/version.cpp
        OUTPUT_NAME npatk
        MODULE
        R2018a)

target_link_libraries(npatk_mex
        lib_npatk)

add_custom_command(TARGET npatk_mex POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:npatk_mex>"
                "${PROJECT_SOURCE_DIR}/matlab/"
        COMMENT "Copying compiled mex function to matlab directory.")