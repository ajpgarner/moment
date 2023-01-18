set(MATLAB_FIND_DEBUG)

find_package(Matlab)

include_directories(${Moment_SOURCE_DIR}/cpp/lib_moment)
include_directories(${Moment_SOURCE_DIR}/cpp/mex_functions)

matlab_add_mex(NAME moment_mex
        SRC
            mex_entry.cpp
            mex_main.cpp
            storage_manager.cpp
            fragments/enumerate_symbols.cpp
            fragments/identify_nonhermitian_elements.cpp
            fragments/identify_nonsymmetric_elements.cpp
            export/export_basis.cpp
            export/export_implicit_symbols.cpp
            export/export_matrix_basis_masks.cpp
            export/export_monomial_rules.cpp
            export/export_operator_matrix.cpp
            export/export_symbol_table.cpp
            import/read_monomial_rules.cpp
            import/read_raw_symbol_matrix.cpp
            import/read_symbol_or_fail.cpp
            utilities/io_parameters.cpp
            utilities/make_sparse_matrix.cpp
            utilities/read_as_scalar.cpp
            utilities/read_as_string.cpp
            utilities/read_as_vector.cpp
            utilities/reflection.cpp
            utilities/reporting.cpp
            functions/alphabetic_name.cpp
            functions/apply_values.cpp
            functions/collins_gisin.cpp
            functions/complete.cpp
            functions/extended_matrix.cpp
            functions/import_matrix.cpp
            functions/function_list.cpp
            functions/generate_basis.cpp
            functions/localizing_matrix.cpp
            functions/moment_matrix.cpp
            functions/new_algebraic_matrix_system.cpp
            functions/new_imported_matrix_system.cpp
            functions/new_inflation_matrix_system.cpp
            functions/new_locality_matrix_system.cpp
            functions/operator_matrix.cpp
            functions/probability_table.cpp
            functions/release.cpp
            functions/rules.cpp
            functions/symbol_table.cpp
            functions/version.cpp
        OUTPUT_NAME mtk
        MODULE
        R2018a)

target_link_libraries(moment_mex
        lib_moment)

add_custom_command(TARGET moment_mex POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:moment_mex>"
                "${PROJECT_SOURCE_DIR}/matlab/"
        COMMENT "Copying compiled mex function to matlab directory.")