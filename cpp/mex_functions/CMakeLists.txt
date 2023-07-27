set(MATLAB_FIND_DEBUG)

find_package(Matlab)

include_directories(${Moment_SOURCE_DIR}/cpp/lib_moment)
include_directories(${Moment_SOURCE_DIR}/cpp/mex_functions)

matlab_add_mex(NAME moment_mex
        SRC
            environmental_variables.cpp
            function_list.cpp
            mex_entry.cpp
            mex_function.cpp
            mex_main.cpp
            eigen/read_eigen_dense.cpp
            eigen/read_eigen_sparse.cpp
            eigen/export_eigen_dense.cpp
            eigen/export_eigen_sparse.cpp
            export/export_basis.cpp
            export/export_collins_gisin.cpp
            export/export_matrix_basis_masks.cpp
            export/export_moment_substitution_rules.cpp
            export/export_operator_matrix.cpp
            export/export_operator_matrix_seq_strings.cpp
            export/export_operator_rules.cpp
            export/export_operator_sequence.cpp
            export/export_osg.cpp
            export/export_polynomial.cpp
            export/export_probability_tensor.cpp
            export/export_symbol_table.cpp
            export/exporter.cpp
            export/full_monomial_specification.cpp
            import/read_measurement_indices.cpp
            import/read_monomial_rules.cpp
            import/read_operator_names.cpp
            import/read_opseq_polynomial.cpp
            import/read_polynomial.cpp
            import/read_raw_symbol_matrix.cpp
            import/read_symbol_or_fail.cpp
            logging/in_memory_logger.cpp
            logging/logger.cpp
            logging/to_file_logger.cpp
            utilities/io_parameters.cpp
            utilities/make_sparse_matrix.cpp
            utilities/read_choice.cpp
            utilities/read_as_scalar.cpp
            utilities/read_as_string.cpp
            utilities/read_as_vector.cpp
            utilities/reflection.cpp
            utilities/reporting.cpp
            functions/conjugate.cpp
            functions/import_matrix.cpp
            functions/generate_basis.cpp
            functions/make_representation.cpp
            functions/release.cpp
            functions/settings.cpp
            functions/simplify.cpp
            functions/suggest_extensions.cpp
            functions/symbol_table.cpp
            functions/transform_symbols.cpp
            functions/word_list.cpp
            functions/debug/echo.cpp
            functions/debug/logging.cpp
            functions/debug/list.cpp
            functions/debug/moment_rule_superset.cpp
            functions/debug/version.cpp
            functions/matrix_system/algebraic_matrix_system.cpp
            functions/matrix_system/imported_matrix_system.cpp
            functions/matrix_system/inflation_matrix_system.cpp
            functions/matrix_system/locality_matrix_system.cpp
            functions/matrix_system/symmetrized_matrix_system.cpp
            functions/moment_rules/apply_moment_rules.cpp
            functions/moment_rules/create_moment_rules.cpp
            functions/moment_rules/moment_rules.cpp
            functions/operator_matrix/substituted_matrix.cpp
            functions/operator_matrix/extended_matrix.cpp
            functions/operator_matrix/localizing_matrix.cpp
            functions/operator_matrix/moment_matrix.cpp
            functions/operator_matrix/operator_matrix.cpp
            functions/operator_rules/complete.cpp
            functions/operator_rules/operator_rules.cpp
            functions/probability/collins_gisin.cpp
            functions/probability/make_explicit.cpp
            functions/probability/probability_table.cpp
            functions/utility/alphabetic_name.cpp
            functions/utility/flatten_indices.cpp
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