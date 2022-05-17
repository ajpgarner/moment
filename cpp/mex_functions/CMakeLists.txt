set(Matlab_ROOT_DIR "C:/Program Files/MATLAB/R2021a")
set(MATLAB_FIND_DEBUG)

find_package(Matlab)

matlab_add_mex(NAME npatk_mex
        SRC
            mex_entry.cpp
            mex_main.cpp
            helpers/read_as_string.cpp
            helpers/reporting.cpp
            helpers/substitute_elements_using_tree.cpp
            helpers/export_substitution_list.cpp
            functions/version.cpp
            functions/make_symmetric.cpp
        OUTPUT_NAME npatk
        R2018a)

target_link_libraries(npatk_mex
        lib_npatk)

