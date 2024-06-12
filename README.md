# Moment
*(c) 2022-2024 Austrian Academy of Sciences*

**Version:** 0.9.0 *(beta)*
 
**Author:** Andrew J. P. Garner

Moment a set of tools designed to aid in the generation and manipulation of operator matrices, for use in the context 
of convex optimization problems. 
In particular, it aids in the generation of hierarchies of moment matrices and 
localizing matrices, such as arise in the NPA and PNA hierarchies. 
Can also generate inflated matrices for classical causal-compatibility scenarios.

## Installation Instructions
The following instructions provided for installing a major release of Moment as a 
precompiled binary. If you wish to produce your own binary (required for all but major 
tagged versions, or for making changes to Moment), instead follow the instructions in 
the [compilation instructions](#compilation-instructions) section below.

First, make sure MATLAB (version 2018a+ / 9.4+) and (at least) one of [YALMIP](https://yalmip.github.io/) 
or [CVX](https://cvxr.com/cvx/) are installed.

Depending on your platform (Windows, GNU/Linux or MacOS) from the releases, download the appropriate:
`moment-0.9.0-win.zip`, `moment-0.9.0-linux.zip`,`moment-0.9.0-mac.zip`, 
and extract the contents to the desired installation location.

From within MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.
This can be done automatically by running the script `mtk_install` from the `moment/matlab` folder, and then restarting MATLAB.
If everything is correctly installed, calling function `mtk` (from any folder) should display the version information of Moment. 


## Compilation Instructions
If you only wish to use a pre-compiled stable release of Moment, instead follow the 
[installation instructions](#installation-instructions) above.
### GNU/Linux
To download and compile the library use the following commands:

<pre>git clone --recursive https://github.com/ajpgarner/moment.git
cd moment
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .</pre>

Once the build is complete, the binary `mtk.mexa64` will be automatically copied into the `moment/matlab` folder. 

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.
This can be done automatically by running the script `mtk_install` from the `moment/matlab` folder, and then restarting MATLAB.
If everything is correctly installed, calling function `mtk` (from any folder) should display the version information of Moment.

### Mac 
Only tested on Intel-based Macs. You first need the compilation tools if you don't already have:

<pre>brew install git
brew install gcc@12
brew install cmake</pre>

It is vital to use GCC 12, it will not work with the native clang. After that the commands are the same as in GNU/Linux:

<pre>git clone --recursive https://github.com/ajpgarner/moment.git
cd moment
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .</pre>

Once the build is complete, the binary `mtk.mexmaci64` will be automatically copied into the `moment/matlab` folder. 

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.
This can be done automatically by running the script `mtk_install` from the `moment/matlab` folder, and then restarting MATLAB.
If everything is correctly installed, calling function `mtk` (from any folder) should display the version information of Moment.

### Windows: CLion
Clone the repository (e.g. with GitHub desktop).

Open CLion, and from the `File > Open` menu, navigate to the cloned repository root directory.

To build the MATLAB binary, select the target `moment_mex` in the Build Configurations drop down, then press the build 
button. Alternatively, select `Build Project` from the `Build` menu.

Once the build is complete, the binary `mtk.mexw64` will have been copied to the `moment/matlab` folder.

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.
This can be done automatically by running the script `mtk_install` from the `moment/matlab` folder, and then restarting MATLAB.
If everything is correctly installed, calling function `mtk` (from any folder) should display the version information of Moment.

### Windows: Visual Studio 
Clone the repository (e.g. with GitHub desktop).

Open Visual Studio 2022, and select "Open a local folder". Navigate to the root of the cloned repository and click 
'Select folder'. Visual Studio will accordingly parse the CMakeList files. The project can then be built by selecting 
"Build All" from Visual Studio's Build menu, or pressing F7.

Once the build is complete, the binary `mtk.mexw64` will have been copied to the `moment/matlab` folder.

To use from MATLAB, add the following folders to the matlab path: `moment/matlab`, `moment/matlab/classes`, and `moment/matlab/functions`.
This can be done automatically by running the script `mtk_install` from the `moment/matlab` folder, and then restarting MATLAB.
If everything is correctly installed, calling function `mtk` (from any folder) should display the version information of Moment.


## Minimum tested versions
MATLAB 2018a (9.4) \
YALMIP R20210331 

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



## Dependencies

### Using Moment
MATLAB 2018a (9.4), or later.

*Either* [YALMIP](https://yalmip.github.io/),
*or* [CVX](https://cvxr.com/cvx/) (version 2.2, not version 3).
 

### Compiling from source
**[Googletest](https://github.com/google/googletest):** C++ unit test suite.

**[Eigen](https://gitlab.com/libeigen/eigen):** Linear algebra template library for C++.

Both dependencies are included as git submodules 
(and hence will be pulled by using the above [compilation instructions](#compilation-instructions)).

If using a pre-compiled binary (and not requiring C++ unit tests), it is not necessary to pull these dependencies.
