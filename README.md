# NPA Toolkit
(c) 2022 Austrian Academy of Sciences
 
Author: Andrew J. P. Garner

NPATK is a set of tools designed to aid in the generation and manipulation of operator matrices, for use in the context 
of convex optimisation problems. In particular, it aids in the generation of hierarchies of moment matrices and 
localizing matrices, such as arise in the NPA [1] and PNA [2] hierarchies.

### Usage

A typical program using NPATK should first define a `Scenario` object, describing the number of distinct parties, the
measurements per parties, and the number of outcomes per measurement. For example, a CHSH Scenario would define two
parties, with two measurements per party, each with two outcomes.

For simple use cases, where only a single moment matrix (i.e. also at a fixed level within the hierarchy), a 
`MomentMatrix` object can be created. For example, 
``scenarioObj.MakeMomentMatrix(1)``
generates the level 1 moment matrix associated with a scenario. 

In more complex cases (e.g. where multiple depths are required, or sets of localizing matrices are also considered), a
`MatrixSystem` should be created. 
This object represents a shared set of SDP variables, where the same variable might appear in multiple different moment 
matrices and localizing matrices.

To ensure consistency between the many dependent objects, once a `MatrixSystem` has been created (even implicitly), no 
further changes can be made to the `Scenario` object. However, a `clone()` method is provided for `Scenario`, that 
constructs a deep copy, which will allow changes to the setting (until it, in turn, is associated with a new 
`MatrixSystem`).



### References / additional reading
**[1]:** *Navascues, Pironio, and Acin*: A convergent hierarchy of semidefinite programs characterizing the set of quantum correlations.\
New J. Phys. 10, 073013 (2008).\
[doi:10.1088/1367-2630/10/7/073013](https://doi.org/10.1088/1367-2630/10/7/073013).

**[2]:** *Pironio,  Navascues, and Acin*: Convergent relaxations of polynomial optimization problems
with non-commuting variables.\
SIAM J. Optim. Volume 20, Issue 5, pp. 2157-2180 (2010).\
[doi:10.1137/090760155](https://doi.org/10.1137/090760155).

## Minimum tested versions
MATLAB 2021b (9.10)

#### For compiling C++ library

*On Windows*: \
Clang 14.0.0 (via clang-cl) or GCC 12.1.0 \
CMAKE 3.22

*On GNU/Linux*: \
GCC 12.1.0 \
CMAKE 3.22



## List of classes, functions and folders
### MATLAB classes

`MatrixSystem`: A collection of matrices, with shared symbols. Usually associated with a `Scenario`. 

`MomentMatrix`: Handle to an individual moment matrix (of particular depth) from within a matrix system.

`RealObject`: Base class for objects, associated with a `Scenario` and `MatrixSystem`, representing things that can be
expressed as linear combination of real-valued elements from a `MomentMatrix`. This includes probabilities, 
normalizations, correlators, etc.

`Scenario`: A description of an experimental setting (including number of parties, measurements for each parties,
number of outcomes for each measurement).

`Scenario.Party`: A spatially isolated agent (e.g. Alice over here, Bob over there). By construction, all matrices generated
respect no-signalling, and so operators associated with different parties are always assumed to commute.


### NPATK functions
The matlab module `npatk` contains the following functions, the names of which should be provided as the first argument
to the call to function `npatk(...)`:

`alphabetic_name`: Converts a numerical index into an alphabetic one.

`complete`: Attempts to complete a set of algebraic rewrite rules.

`collins_gisin`: Returns list of symbols in a matrix system, as a matrix using Collins-Gisin indexing. 

`generate_basis`: Provide symmetric or Hermitian basis matrices for a given symbolic matrix.

`localizing_matrix`: Generates a localizing matrix for a supplied set of Hermitian operators and monomial expression.

`make_hermitian`: Makes a symbolic matrix Hermitian, by inferring equality constraints and applying them.

`make_symmetric`: Makes a symbolic matrix symmetric, by inferring equality constraints and applying them.

`moment_matrix`: Generates a moment matrix for a supplied set of Hermitian operators.

`new_algebraic_matrix_system`: Starts a new context of shared variables, with specified algebraic relations.

`new_inflation_matrix_system`: Starts a new context of shared variables, for an inflated causal network.

`new_locality_matrix_system`: Starts a new context of shared variables, for locality (party, measurement, outcome) 
settings.

`rules`: Returns a ruleset associated with an algebraic matrix system.

`operator_matrix`: Returns an operator matrix from a matrix system. 

`symbol_table`: Returns total list of operators in a matrix system.

`probability_table`: Calculates the coefficients of all implied outcomes from the moment matrix / matrix system.

`release`: Frees the internally-stored object at the supplied key.

`version`: Returns the version of this software.


### Directory structure

`\cpp`: Root of the C++ source code.

`\cpp\googletest`: Inclusion of googletest subproject

`\cpp\lib_npatk`: Toolkit algorithms agnostic of MATLAB. Builds `lib_npatk`.

`\cpp\lib_npatk\operators`: Aspects of `lib_npatk` specifically concerned with Hermitian operator manipulation.

`\cpp\lib_npatk\operators\matrix`: Aspects of `lib_npatk` specifically concerned with systems of moment matrices, localizing 
matrices, etc.

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

`\matlab\examples`: Example scripts making use of the NPATK library.

`\matlab\tests`: MATLAB unit tests for `npatk` function.

`\prototype` *(temporary)*: Sketch implementations of algorithms in MATLAB.
