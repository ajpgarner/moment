# Moment
(c) 2022-2023 Austrian Academy of Sciences
 
Author: Andrew J. P. Garner

Moment a set of tools designed to aid in the generation and manipulation of operator matrices, for use in the context 
of convex optimisation problems. In particular, it aids in the generation of hierarchies of moment matrices and 
localizing matrices, such as arise in the NPA [1] and PNA [2] hierarchies.

### Usage

A typical program using Moment should first define a `Scenario` object, describing the number of distinct parties, the
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
Clang 14.0.0 (via clang-cl), GCC 12.1.0, or Visual Studio 2022 \
CMAKE 3.22

*On GNU/Linux*: \
GCC 12.1.0 \
CMAKE 3.22

## Installation Instructions
### GNU/Linux
To download and compile the library use the following commands:

<pre>git clone https://github.com/ajpgarner/moment.git
cd moment
cmake .
cmake --build .</pre>

Once the build is complete, the binary `mtk.mexa64` will be automatically copied into the `moment/matlab` folder. 

To use from MATLAB, add the following folder to the matlab path: `moment/matlab`

To build with the C++ unit tests, instead:

<pre>git clone https://github.com/ajpgarner/moment.git
cd moment
git submodule init
git submodule update
cmake .
cmake --build .</pre>

### Windows: CLion
Clone the repository (e.g. with GitHub desktop).

Open CLion, and from the `File > Open` menu, navigate to the cloned repository root directory.

To build the MATLAB binary, select the target `moment_mex` in the Build Configurations drop down, then press the build 
button. Alternatively, select `Build Project` from the `Build` menu.

Once the build is complete, the binary `mtk.mexw64` will have been copied to the `moment/matlab` folder.

To use from MATLAB, add the following folder to the matlab path: `moment/matlab`.

### Windows: Visual Studio 
Clone the repository (e.g. with GitHub desktop).

Open Visual Studio 2022, and select "Open a local folder". Navigate to the root of the cloned repository and click 
'Select folder'. Visual Studio will accordingly parse the CMakeList files. The project can then be built by selecting 
"Build All" from Visual Studio's Build menu, or pressing F7.

Once the build is complete, the binary `mtk.mexw64` will have been copied to the `moment/matlab` folder.

To use from MATLAB, add the following folder to the matlab path: `moment/matlab`.


## List of classes, functions and folders
### MATLAB classes

`MatrixSystem`: A collection of matrices, with shared symbols. Usually associated with a `Scenario`. 

`MomentMatrix`: Handle to an individual moment matrix (of particular depth) from within a matrix system.

`RealObject`: Base class for objects, associated with a `Scenario` and `MatrixSystem`, representing things that can be
expressed as linear combination of real-valued elements from a `MomentMatrix`. This includes probabilities, 
normalizations, correlators, etc.

`LocalityScenario`: A description of an experimental setting (including number of parties, measurements for each party,
number of outcomes for each measurement).

`LocalityScenario.Party`: A spatially isolated agent (e.g. Alice over here, Bob over there). By construction, all matrices generated
respect no-signalling, and so operators associated with different parties are always assumed to commute.


### Moment functions
The matlab module `mtk` contains the following functions, the names of which should be provided as the first argument
to the call to function `mtk(...)`:

`alphabetic_name`: Converts a numerical index into an alphabetic one.

`apply_values`: Create a new matrix by substituting listed symbols with numeric values.

`complete`: Attempts to complete a set of algebraic rewrite rules.

`collins_gisin`: Returns list of symbols in a matrix system, as a matrix using Collins-Gisin indexing. 

`extended_matrix`: Creates a scalar extension of a moment matrix, extended by listed symbols.

`generate_basis`: Provide symmetric or Hermitian basis matrices for a given symbolic matrix.

`import_matrix`: Adds a symbol matrix manually to an imported matrix system.

`list`: Lists the matrices stored in a given matrix system.

`localizing_matrix`: Generates a localizing matrix for a supplied set of Hermitian operators and monomial expression.

`make_explicit`: Converts a full probability distribution into a list of explicit symbol assignments for the locality 
and inflation scenarios. 

`moment_matrix`: Generates a moment matrix for a supplied set of Hermitian operators.

`new_algebraic_matrix_system`: Starts a new context of shared variables, with specified algebraic relations.

`new_imported_matrix_system`: Starts a new context of shared variables, for matrices manually entered via
`import_matrix`.

`new_inflation_matrix_system`: Starts a new context of shared variables, for an inflated causal network.

`new_locality_matrix_system`: Starts a new context of shared variables, for locality (party, measurement, outcome) 
settings.

`probability_table`: Calculates the coefficients of all implied outcomes from the moment matrix / matrix system.

`operator_matrix`: Returns an operator matrix from a matrix system. 

`release`: Frees the internally-stored object at the supplied key.

`rules`: Returns a ruleset associated with an algebraic matrix system.

`settings`: Adjust environmental variables for Moment.

`suggest_extensions`: Returns list of symbols that could be used to extend a matrix to impose factorization constraints. 

`symbol_table`: Returns total list of operators in a matrix system.

`version`: Returns the version of this software.


### Directory structure

`\cpp`: Root of the C++ source code.

`\cpp\googletest`: Inclusion of googletest subproject

`\cpp\lib_moment`: Toolkit algorithms agnostic of MATLAB. Builds `lib_moment`.

`\cpp\lib_moment\matrix`: Aspects of `lib_moment` concerning operator matrices.

`\cpp\lib_moment\scenarios`: Aspects of `lib_moment` concerning functionality shared between various matrix
systems.

`\cpp\lib_moment\scenarios\algebraic`: Aspects of `lib_moment` concerning generic algebraic operator
manipulation (with monomial rewrite rules).

`\cpp\lib_moment\scenarios\imported`: Aspects of `lib_moment` concerning manually imported matrices.

`\cpp\lib_moment\scenarios\inflation`: Aspects of `lib_moment` concerning inflation scenarios.

`\cpp\lib_moment\scenarios\locality`: Aspects of `lib_moment` concerning locality scenarios (e.g. NPA Hierarchy).

`\cpp\lib_moment\operators\matrix`: Aspects of `lib_moment` concerning systems of moment matrices,
localizing matrices, etc.

`\cpp\lib_moment\symbolic`: Aspects of `lib_moment`  concerning symbolic expressions.

`\cpp\lib_moment\utilities`: General boilerplate code for `lib_moment`, not entirely specific to the manipulation of 
SDP hierarchies.

`\cpp\mex_functions`: Root for MATLAB/C++ interface code (i.e. building `mtk` mex library).

`\cpp\mex_functions\export`: Code pieces specific to Moment, involving the conversion of Moment library objects 
to MATLAB arrays.

`\cpp\mex_functions\functions`: Entry points for the various `mtk` commands.

`\cpp\mex_functions\import`: Code pieces specific to Moment, involving the reading of MATLAB arrays into Moment
library objects.

`\cpp\mex_functions\utilities`: Boilerplate code for interfacing MATLAB with C++, not specific to Moment.

`\cpp\tests`: C++ unit tests for `lib_moment` (build using googletest).

`\cpp\tests\operators`: C++ unit tests for Hermitian-operator manipulation.

`\cpp\tests\scenarios`: C++ unit tests for specific matrix system types.

`\cpp\tests\scenarios\algebraic`: C++ unit tests for algebraic matrix systems.

`\cpp\tests\scenarios\imported`: C++ unit tests for imported matrix systems.

`\cpp\tests\scenarios\inflation`: C++ unit tests for inflation scenario matrix systems.

`\cpp\tests\scenarios\locality`: C++ unit tests for locality scenario matrix systems.

`\cpp\tests\symbolic`: C++ unit tests for symbolic manipulation.

`\cpp\tests\utilities`: C++ unit tests for general utility functions.

`\matlab`: Root of Moment MATLAB classes & functions.

`\matlab\+Algebraic`: MATLAB classes associated with generic algebraic scenarios.

`\matlab\+Inflation`: MATLAB classes associated with inflation scenarios.

`\matlab\+Locality`: MATLAB classes associated with locality scenarios.

`\matlab\+Util`: Minor utility functions for MATLAB. 

`\matlab\examples`: Example scripts making use of the Moment library.

`\matlab\tests`: MATLAB unit tests for `moment` mex library, and other matlab classes.