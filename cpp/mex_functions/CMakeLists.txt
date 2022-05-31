set(Matlab_ROOT_DIR "C:/Program Files/MATLAB/R2021a")
set(MATLAB_FIND_DEBUG)

find_package(Matlab)

include_directories(${NPAToolKit_SOURCE_DIR}/cpp/lib_npatk)
include_directories(${NPAToolKit_SOURCE_DIR}/cpp/mex_functions)

matlab_add_mex(NAME npatk_mex
        SRC
            mex_entry.cpp
            mex_main.cpp
            fragments/enumerate_symbols.cpp
            fragments/export_basis_key.cpp
            fragments/export_substitution_list.cpp
            fragments/substitute_elements_using_tree.cpp
            fragments/read_symbol_or_fail.cpp
            utilities/read_as_string.cpp
            utilities/reporting.cpp
            functions/generate_basis.cpp
            functions/make_symmetric.cpp
            functions/version.cpp
        OUTPUT_NAME npatk
        R2018a)

target_link_libraries(npatk_mex
        lib_npatk)

add_custom_command(TARGET npatk_mex POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:npatk_mex>"
                "${PROJECT_SOURCE_DIR}/matlab/"
        COMMENT "Copying to matlab directory.")