set(Matlab_ROOT_DIR "C:/Program Files/MATLAB/R2021a")
set(MATLAB_FIND_DEBUG)

find_package(Matlab)

add_library(lib_mexhelpers
        helpers/reporting.cpp
        helpers/substitute_elements_using_tree.cpp
        helpers/export_substitution_list.cpp)

target_include_directories(lib_mexhelpers PRIVATE ${Matlab_INCLUDE_DIRS})

matlab_add_mex(NAME impose_symmetric_constraints
        SRC impose_symmetric_constraints.cpp
        R2018a)

target_link_libraries(impose_symmetric_constraints
        lib_npatk
        lib_mexhelpers)

