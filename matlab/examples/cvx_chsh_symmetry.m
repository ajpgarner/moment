%% Example: cvx_chsh_symmetry.m
% Demonstrates Ioannou-Rosset symmetry reduction on the CHSH scenario.
% Expected answer: 2 sqrt 2 (2.828...)

%% Settings
mm_level = 3;

%% Set up base scenario (CHSH)
chsh_scenario = LocalityScenario(2, 2, 2);

%% Define generators of symmetries of the CHSH inequality
chsh_generators = {[[1 0 1 0 0];
                    [0 1 0 0 0];
                    [0 0 -1 0 0];
                    [0 0 0 0 1];
                    [0 0 0 1 0]], ...
                   [[1 0 0 0 0];
                    [0 0 0 1 0];
                    [0 0 0 0 1];
                    [0 1 0 0 0];
                    [0 0 1 0 0]]};
                
%% Set up symmetrized scenario
tic
sym_scenario = SymmetrizedScenario(chsh_scenario, ...
                                   chsh_generators, ...
                                   'word_length', 2*mm_level);
sym_scenario.System;                               
sym_time = toc;
fprintf("Generated symmetrized scenario in %f seconds.\n", toc);

%% Make moment matrices
base_mm = chsh_scenario.MomentMatrix(mm_level);
sym_mm = sym_scenario.MomentMatrix(mm_level);
mtk('list','verbose');
disp(base_mm.SequenceStrings);
disp(sym_mm.SequenceStrings)

%% Make CHSH inequality from full-correlator
CHSH_ineq = chsh_scenario.FCTensor([[0 0 0]; [0 1 1]; [0 1 -1]]);
sym_CHSH_ineq = sym_scenario.Transform(CHSH_ineq);

%% Solve SDP
cvx_begin sdp
    % Declare CVX variables for symmetrized system
    sym_scenario.cvxVars('a', 'b');
    
    % Compose moment matrix from these basis variables
    M = sym_mm.cvx(a, b);
    
    % Constraints (normalization & positivity)
    a(1) == 1;
    M >= 0;
    
    % Optimize equality
    solve_chsh_ineq = sym_CHSH_ineq.cvx(a);
    maximize(solve_chsh_ineq);
cvx_end
