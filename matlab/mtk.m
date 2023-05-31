%% MTK - Moment (Tool Kit).
% (c) 2022-2023 Austrian Academy of Sciences
%  Author: Andrew J. P. Garner
%
% This mex file contains Moment's C++ library.
%
% GENERAL SYNTAX: 
%
%      [output1, ... outputM] = mtk('function_name', input1, ..., inputN)
%
% FUNCTIONS:
%
% algebraic_matrix_system
%       Creates context of shared variables, with algebraic relations.
% 
% alphabetic_name 
%       Converts a numerical index into an alphabetic one.
% 
% collins_gisin
%       Returns list of symbols in a matrix system as a matrix, using
%       Collins-Gisin indexing.
% 
% complete 
%       Attempts to complete a set of algebraic rewrite rules.
% 
% conjugate
%       Returns the complex-conjugation of an operator sequence.
% 
% create_moment_rules
%       Creates polynomial rewrite rules, acting at the level of moments.
%
% echo [DEBUG]
%       Returns supplied numerical array; tests MATLAB to Eigen interface.
% 
% extended_matrix
%       Creates a moment matrix, scalar-extended by listed symbols.
% 
% generate_basis
%       Gets basis matrices for the specified matrix.
% 
% import_matrix
%       Adds a symbol matrix manually to an imported matrix system.
%
% imported_matrix_system
%       Creates context of shared variables, for matrices manually input 
%       via 'import_matrix'.
% 
% inflation_matrix_system
%       Creates context of shared variables, for inflated causal network.
% 
% list [DEBUG]
%       Lists the matrices stored in a given matrix system.
% 
% localizing_matrix
%       Generates a localizing matrix for the supplied monomial expression.
%
% locality_matrix_system
%       Creates context of shared variables, for locality (party, 
%       measurement, outcome) settings.
% 
% make_explicit
%       Converts a full probability distribution into a list of explicit 
%       symbol assignments for the locality and inflation scenarios. 
% 
% moment_matrix
%       Generates a moment matrix for supplied set of Hermitian operators.
% 
% monomial_rules [DEBUG]
%       Returns the ruleset associated with an algebraic matrix system.
%
% probability_table
%       Calculates all implied outcomes as sum of explicit operator 
%       sequences found in the matrix system.
% 
% operator_matrix
%       Returns an operator matrix from a matrix system. 
% 
% release
%       Frees the internally-stored object at the supplied key.
% 
% settings
%       Adjust environmental variables for Moment.
% 
% simplify
%       Returns a simplified "canonical form" of an operator sequence.
% 
% substituted_matrix
%       Creates new matrix, substituting symbols according to moment rules.
%
% suggest_extensions
%       Returns list of symbols that could be used to extend a matrix to 
%       impose factorization constraints. 
% 
% symbol_table
%       Returns total list of operators in a matrix system.
% 
% symmetrized_matrix_system
%       Creates context of shared variables by applying symmetries to an
%       existing context.
%
% version
%       Returns the compiled version of this software.
% 
% word_list
%       Returns all unique monomial sequences up to a given length.
%

%% Error if invoked
error("This script should never be executed, as it should be shadowed by Moment's mex file!" + newline + ... 
      "If you are reading this error message, it is likely that the mtk.mexa64/mtk.mexw64 executable has not yet been compiled." + newline + ...
      "See ../README.md for instructions on how to do this.");