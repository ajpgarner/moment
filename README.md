# NPA Toolkit
(c) 2022 Austrian Academy of Sciences
 
Author: Andrew J.P. Garner

A set of tools designed for straightforward manipulation of convex optimisation problems involving moment matrices,
such as the NPA hierarchy. 

### NPATK functions
The matlab module `npatk` contains the following functions, the names of which should be provided as the first argument
to the call to function `npatk(...)`:

`alphabetic_name`: Converts a numerical index into an alphabetic one.

`generate_basis`: Provide symmetric or Hermitian basis matrices for a given symbolic matrix.

`make_hermitian`: Makes a symbolic matrix Hermitian, by inferring equality constraints and applying them.

`make_moment_matrix`: Generates a moment matrix for a supplied set of Hermitian operators.

`make_symmetric`: Makes a symbolic matrix symmetric, by inferring equality constraints and applying them.

`probability_table`: Calculates the coefficients of all implied outcomes from the moment matrix.

`release`: Frees the internally-stored object at the supplied key.

`version`: Returns the version of this software.


### Directory structure

`\cpp`: Root of the C++ source code.

`\cpp\googletest`: Inclusion of googletest subproject

`\cpp\lib_npatk`: Toolkit algorithms agnostic of MATLAB. Builds `lib_npatk`.

`\cpp\lib_npatk\operators`: Aspects of `lib_npatk` specifically concerned with Hermitian operator manipulation, moment
 matrices and the like.

`\cpp\lib_npatk\symbolic`: Aspects of `lib_npatk` specifically concerned with simplification of symbolic expressions.

`\cpp\lib_npatk\utilities`: General boilerplate code for `lib_npatk`, not entirely specific to the manipulation of 
SDP hierarchies.

`\cpp\mex_functions`: Source root for MATLAB code (i.e. building `npatk` mex function).

`\cpp\mex_functions\fragments`: Code pieces specific to NPATK, involving manipulation of
MATLAB arrays, that are used by more than one mex function.

`\cpp\mex_functions\functions`: Entry points for the various `npatk` commands.

`\cpp\mex_functions\matlab_classes`: Mirrors of matlab classes (e.g. defined in `\matlab`), so as to be parsed into C++.

`\cpp\mex_functions\utilities`: Boilerplate code, useful for interfacing MATLAB with C++.

`\cpp\tests`: C++ unit tests for `lib_npatk` (build using googletest).

`\cpp\tests\operators`: C++ unit tests specifically for Hermitian-operator manipulation aspects of `lib_npatk`.

`\matlab`: Root of NPATK MATLAB classes & functions

`\matlab\tests`: MATLAB unit tests for `npatk` function.

`\prototype` *(temporary)*: Sketch implementations of algorithms in MATLAB.