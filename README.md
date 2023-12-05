# Moment
(c) 2022-2023 Austrian Academy of Sciences
 
Author: Andrew J. P. Garner

Moment a set of tools designed to aid in the generation and manipulation of operator matrices, for use in the context 
of convex optimization problems. 
In particular, it aids in the generation of hierarchies of moment matrices and 
localizing matrices, such as arise in the NPA and PNA hierarchies. 
Can also generate matrices for inflated causal-compatibility scenarios.

## Minimum tested versions
MATLAB 2018a (9.4)

#### For compiling C++ library

*On Windows*: \
Clang 14.0.0 (via clang-cl), GCC 12.1.0, or Visual Studio 2022 \
CMAKE 3.22

*On GNU/Linux*: \
GCC 12.3.0 \
CMAKE 3.22

*On Mac*: \
GCC 12.3.0 \
CMAKE 3.22


## Installation Instructions
### GNU/Linux
To download and compile the library use the following commands:

<pre>git clone --recursive https://github.com/ajpgarner/moment.git
cd moment
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . -j 17</pre>

Once the build is complete, the binary `mtk.mexa64` will be automatically copied into the `moment/matlab` folder. 

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.

### Mac 
Only tested on Intel-based Macs. You first need the compilation tools if you don't already have:

<pre>brew install git
brew install gcc@12
brew install cmake</pre>

It is vital to use GCC 12, it will not work with the native clang. After that the commands are the same as in GNU/Linux:

<pre>git clone --recursive https://github.com/ajpgarner/moment.git
cd moment
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . -j 17</pre>

Once the build is complete, the binary `mtk.mexmaci64` will be automatically copied into the `moment/matlab` folder. 

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.

### Windows: CLion
Clone the repository (e.g. with GitHub desktop).

Open CLion, and from the `File > Open` menu, navigate to the cloned repository root directory.

To build the MATLAB binary, select the target `moment_mex` in the Build Configurations drop down, then press the build 
button. Alternatively, select `Build Project` from the `Build` menu.

Once the build is complete, the binary `mtk.mexw64` will have been copied to the `moment/matlab` folder.

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.

### Windows: Visual Studio 
Clone the repository (e.g. with GitHub desktop).

Open Visual Studio 2022, and select "Open a local folder". Navigate to the root of the cloned repository and click 
'Select folder'. Visual Studio will accordingly parse the CMakeList files. The project can then be built by selecting 
"Build All" from Visual Studio's Build menu, or pressing F7.

Once the build is complete, the binary `mtk.mexw64` will have been copied to the `moment/matlab` folder.

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.

## Dependencies
**[Googletest](https://github.com/google/googletest):** C++ unit test suite.

**[Eigen](https://gitlab.com/libeigen/eigen):** Linear algebra template library for C++.

Both dependencies are included as git submodules (and hence will be pulled by using the above installation instructions).


## List of classes, functions and folders
### Moment functions
The matlab module `mtk` defines a set of functions, the names of which should be provided as the first argument
to the call to function `mtk(...)`. For information about these, type `help mtk` from MATLAB, or read the file 
`matlab/mtk.m`

### Directory outline

`\`: Root of Moment.

`\cpp`: Root of the C++ source code.

`\cpp\eigen`: Inclusion of [Eigen](https://gitlab.com/libeigen/eigen) subproject.

`\cpp\googletest`: Inclusion of [googletest](https://github.com/google/googletest) subproject.

`\cpp\lib_moment`: Toolkit algorithms agnostic of MATLAB. Builds `lib_moment`.

`\cpp\mex_functions`: Root for MATLAB/C++ interface code (i.e. building `mtk` mex library).

`\cpp\stress_tests`: Stand-alone benchmarking tests.

`\cpp\tests`: C++ unit tests for `lib_moment` (build using googletest).

`\matlab`: Root of Moment MATLAB package

`\matlab\classes`: Object-oriented interface for moment.

`\matlab\examples`: Example scripts making use of the Moment library.

`\matlab\functions`: MATLAB functional interface for Moment.

`\matlab\tests`: MATLAB unit tests for `moment` mex library, and other matlab classes.
