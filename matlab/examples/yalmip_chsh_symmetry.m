%% Example: yalmip_chsh_symmetry.m
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
sym_scenario = SymmetrizedScenario(chsh_scenario, ...
                                   chsh_generators, ...
                                   'word_length', 2*mm_level);

%% Make moment matrices
base_mm = chsh_scenario.MomentMatrix(mm_level);
sym_mm = sym_scenario.MomentMatrix(mm_level);

%% Make CHSH inequality from full-correlator
CHSH_ineq = chsh_scenario.FCTensor([[0 0 0]; [0 1 1]; [0 1 -1]]);
sym_CHSH_ineq = sym_scenario.Transform(CHSH_ineq);

%% Define and solve SDP
yalmip('clear')

% Get SDP vars and matrix
[a, b] = sym_scenario.yalmipVars();
M = sym_mm.Apply(a, b);

% Constraints (normalization, positivity)
constraints = [a(1) == 1];
constraints = [constraints, M>=0];

% Objective function (maximize)
objective = -sym_CHSH_ineq.Apply(a, b);

% Solve
optimize(constraints, objective); 

%% Get solutions
a_vals = value(a);
disp(sym_CHSH_ineq.Apply(a_vals));

