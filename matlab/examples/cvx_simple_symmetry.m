%% Example: cvx_simple_symmetry.m
% Demonstrates Ioannou-Rosset symmetry reduction on a trivial scenario
% Expected answer: +1

%% Settings
mm_level = 1;

%% Set up base scenario (CHSH)
base_scenario = AlgebraicScenario(['x', 'y']);

%% Define generators of symmetries of the CHSH inequality
z2_generators = {[[1 0 0];
                  [0 0 1];
                  [0 1 0]]};
                
%% Set up symmetrized scenario
sym_scenario = SymmetrizedScenario(base_scenario, ...
                                   z2_generators, ...
                                   'word_length', 2*mm_level);
sym_scenario.System;                               

%% Make moment matrices
base_mm = base_scenario.MomentMatrix(mm_level);
sym_mm = sym_scenario.MomentMatrix(mm_level);
%
mtk('list','verbose');
disp(base_mm.SequenceStrings);
disp(sym_mm.SequenceStrings)

%% Make CHSH inequality from full-correlator
[x, y] = base_scenario.getAll();
base_objective = x + y;
base_constraint = x + y - 1;
sym_objective = sym_scenario.Transform(base_objective);
sym_constraint = sym_scenario.Transform(base_constraint);

%% Solve SDP
cvx_begin sdp
     % Declare CVX variables for symmetrized system
     sym_scenario.cvxVars('a');
     
     % Compose moment matrix from these basis variables
     M = sym_mm.Apply(a);
     
     % Constraints (normalization & positivity)
     a(1) == 1;
     M >= 0;
     sym_constraint.Apply(a) >= 0;
     
     % Optimize equality
     solve_obj = sym_objective.Apply(a);
     minimize(solve_obj);
cvx_end
