%% Example: yalmip_heisenberg.m
% Calculates bounds on the ground state energy of a symmetric Heisenberg 
% chain, using state-optimality conditions [arXiv:2311.18707].
%
% Expected results: [-0.4671, −0.4462] for N=6; analytic result: −0.4671.
%  (see table II of arXiv:2311.18707 for other values of N).

%% Parameters
chain_length = 6;
mm_level = 3;
lm_level = 3;
so_level = 2; % State-optimality commutator conditions
% X_i X_{i+1}, Y_i Y_{i+1} and Z_i Z_{i+1} terms in Hamiltonian:
H_coupling = [0.25, 0.25, 0.25]; 
wrap = true; % Treat qubit N as neighbouring qubit 1?
neighbours = 1; % Limits matrix top row to number of nearest neighbours.

%% Set up scenario
setup_start_time = tic;
setting = PauliScenario(chain_length, 'wrap', wrap, 'symmetrized', true);
[X, Y, Z] = setting.getAll();

% Build coefficients into Hamiltonian polynomial
base_H = H_coupling(1)*X(1)*X(2) ...
       + H_coupling(2)*Y(1)*Y(2) ...
       + H_coupling(3)*Z(1)*Z(2);
H = setting.symmetrize(base_H);

% Generate operator matrices
raw_mm = setting.MomentMatrix(mm_level, neighbours);
raw_lm_H = setting.LocalizingMatrix(H, lm_level, neighbours);
raw_am_H = setting.AnticommutatorMatrix(-0.5*H, lm_level, neighbours);
raw_gamma = raw_lm_H + raw_am_H;

% Get commutating constraints...
monomials = setting.WordList(so_level, neighbours);
linear = commutator(monomials, H);

moment_rules = MomentRulebook(setting, "Commutator constraints");
moment_rules.Add(linear, false);
mm = raw_mm.ApplyRules(moment_rules);
gamma = raw_gamma.ApplyRules(moment_rules);

setup_time = toc(setup_start_time);
fprintf("Generated moment objects in %f seconds.\n", setup_time);

%% Define SDP using Yalmip
a = setting.yalmipVars(); % NB: PauliScenario has no complex basis.
M = mm.Apply(a);
G = gamma.Apply(a);
G = G(2:end, 2:end);

objective = H.Apply(a);
constraints = [a(1) == 1, M >= 0, G >= 0];

%% Solve for upper bound (maximize):
ops = sdpsettings(sdpsettings, 'verbose', 1);
optimize(constraints, -objective, ops);
upper_bound = value(objective);

%% Solve for lower bound (minimize):
ops = sdpsettings(sdpsettings, 'verbose', 1);
optimize(constraints, objective, ops);
lower_bound = value(objective);

%% Display results
fprintf("Lower bound: %g\n", lower_bound);
fprintf("Upper bound: %g\n", upper_bound);
