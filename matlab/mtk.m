% MTK - Moment (Tool Kit).
% (c) 2022-2023 Austrian Academy of Sciences
%
% This mex file is Moment's C++ library.
%
% GENERAL SYNTAX: 
%
%      [output1, ... outputM] = mtk('function_name', param1, ..., paramN)
%
% FUNCTIONS:
%
% add_symmetry
%       Associate a symmetry group with a context.
% 
% alphabetic_name 
%       Converts a numerical index into an alphabetic one.
% 
% apply_values
%       Creates new matrix substituting listed symbols with numeric values.
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
% echo [Debug]
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
% list
%       Lists the matrices stored in a given matrix system.
% 
% localizing_matrix
%       Generates a localizing matrix for the supplied monomial expression.
% 
% make_explicit
%       Converts a full probability distribution into a list of explicit 
%       symbol assignments for the locality and inflation scenarios. 
% 
% moment_matrix
%       Generates a moment matrix for supplied set of Hermitian operators.
% 
% new_algebraic_matrix_system
%       Creates context of shared variables, with algebraic relations.
% 
% new_imported_matrix_system
%       Creates context of shared variables, for matrices manually input 
%       via 'import_matrix'.
% 
% new_inflation_matrix_system
%       Creates context of shared variables, for inflated causal network.
% 
% new_locality_matrix_system
%       Createscontext of shared variables, for locality (party, 
%       measurement, outcome) settings.
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
% rules
%       Returns a ruleset associated with an algebraic matrix system.
% 
% settings
%       Adjust environmental variables for Moment.
% 
% simplify
%       Returns a simplified "canonical form" of an operator sequence.
% 
% suggest_extensions
%       Returns list of symbols that could be used to extend a matrix to 
%       impose factorization constraints. 
% 
% symbol_table
%       Returns total list of operators in a matrix system.
% 
% version
%       Returns the compiled version of this software.
% 
% word_list
%       Returns all unique monomial sequences up to a given length.
%