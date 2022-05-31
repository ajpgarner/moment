# NPA Toolkit
(c) 2022 Austrian Academy of Sciences
 
Author: Andrew J.P. Garner

A set of tools designed for straightforward manipulation of convex optimisations, to use
with `yalmip` and `cvx`.


### Directory structure

`\cpp`: Root of the C++ source code.

`\cpp\googletest`: Inclusion of googletest subproject

`\cpp\lib_npatk`: Toolkit algorithms agnostic of MATLAB. Builds `lib_npatk`.

`\cpp\mex_functions`: Source root for MATLAB code (i.e. building `npatk` mex function).

`\cpp\mex_functions\fragments`: Code pieces specific to NPATK, involving manipulation of
MATLAB arrays, that are used by more than one mex function.

`\cpp\mex_functions\functions`: Entry points for the various `npatk` commands.

`\cpp\mex_functions\utilities`: Boilerplate code, useful for interfacing MATLAB with C++.

`\cpp\tests`: C++ unit tests for `lib_npatk` (build using googletest).

`\matlab`: Root of NPATK MATLAB scripts & functions

`\matlab\tests`: MATLAB unit tests for `npatk` function.

`\prototype` *(temporary)*: Sketch implementations of algorithms in MATLAB.