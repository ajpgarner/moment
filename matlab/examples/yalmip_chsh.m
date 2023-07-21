%% Example: yalmip_chsh.m 
% Solves CHSH scenario, with yalmip.
%

%% Parameters
mm_level = 3;

%% Set up scenario
% Two parties
setting = LocalityScenario(2);
Alice = setting.Parties(1);
Bob = setting.Parties(2);

% Each party with two measurements
A0 = Alice.AddMeasurement(2);
A1 = Alice.AddMeasurement(2);
B0 = Bob.AddMeasurement(2);
B1 = Bob.AddMeasurement(2);

%% Make matrices and polynomials
% Make moment matrix
matrix = setting.MomentMatrix(mm_level);

% Make correlator objects
Corr00 = A0.Correlator(B0);
Corr01 = A0.Correlator(B1);
Corr10 = A1.Correlator(B0);
Corr11 = A1.Correlator(B1);

% Make CHSH object
CHSH_ineq = Corr00 + Corr01 + Corr10 - Corr11;

%% Define and solve SDP
yalmip('clear')

% Get SDP vars and matrix
a = setting.yalmipVars();
M = matrix.yalmip(a);

% Constraints (normalization, positivity)
constraints = [a(1) == 1];
constraints = [constraints, M>=0];

% Objective function (maximize)
objective = -CHSH_ineq.yalmip(a);

% Solve
optimize(constraints, objective); 

%% Get solutions
a_vals = value(a);
disp(setting.FullCorrelator.Apply(a_vals));
disp(CHSH_ineq.Apply(a_vals));
