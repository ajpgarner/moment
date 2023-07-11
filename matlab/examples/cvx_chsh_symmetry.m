%% Example: cvx_chsh_symmetry.m
% Demonstrates Ioannou-Rosset symmetry reduction on the CHSH scenario.

clear
clear mtk

% Settings
mm_level = 6;

% Base scenario
chsh_scenario = LocalityScenario(2, 2, 2);

% Generators of symmetries of the CHSH inequality
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
                
% Symmetrized scenario
sym_scenario = SymmetrizedScenario(chsh_scenario, chsh_generators, 2*mm_level);

% Get moment matrices
base_mm = chsh_scenario.MomentMatrix(mm_level);
sym_mm = sym_scenario.MomentMatrix(mm_level);


% Show systems, and generated matrices
mtk('list','verbose');
disp(base_mm.SymbolMatrix);
disp(sym_mm.SymbolMatrix)

% Make CHSH inequality from full-correlator
CHSH_ineq = chsh_scenario.FCTensor([[0 0 0]; [0 1 1]; [0 1 -1]]);
sym_CHSH_ineq = sym_scenario.Transform(CHSH_ineq);

cvx_begin sdp
    % Declare CVX variables for symmetrized system
    sym_mm.cvxVars('a', 'b');
    
    % Compose moment matrix from these basis variables
    M = sym_mm.cvxComplexMatrix(a, b);
    
    % Constraints (normalization & positivity)
    a(1) == 1
    M >= 0
    
    % Optimize equality
    solve_chsh_ineq = sym_CHSH_ineq.cvx(a);
    maximize(solve_chsh_ineq);
cvx_end
