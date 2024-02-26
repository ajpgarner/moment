%% MTK - Moment (Tool Kit).
% (c) 2022-2024 Austrian Academy of Sciences
%  Author: Andrew J. P. Garner
%
% This mex file contains Moment's C++ library.
%
% GENERAL SYNTAX: 
%
%      [output1, ... outputM] = mtk('function_name', input1, ..., inputN)
%

%% Error if invoked
error("This script should never be executed, as it should be shadowed by Moment's mex file!" + newline + ... 
      "If you are reading this error message, it is likely that the mtk.mexa64/mtk.mexw64 executable has not yet been compiled." + newline + ...
      "See ../README.md for instructions on how to do this.");