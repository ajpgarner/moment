% EXAMPLE: CHSH scenario, using yalmip
%
addpath('..')
yalmip('clear')
clear
clear mtk;

% Two parties
chsh = LocalityScenario(2);
Alice = chsh.Parties(1);
Bob = chsh.Parties(2);

% Each party with two measurements
A0 = Alice.AddMeasurement(2);
A1 = Alice.AddMeasurement(2);
B0 = Bob.AddMeasurement(2);
B1 = Bob.AddMeasurement(2);

% Make moment matrix
matrix = chsh.MomentMatrix(1);

% Make correlator objects
Corr00 = A0.Correlator(B0);
Corr01 = A0.Correlator(B1);
Corr10 = A1.Correlator(B0);
Corr11 = A1.Correlator(B1);

% Make CHSH object
CHSH_ineq = Corr00 + Corr01 + Corr10 - Corr11;

% Get SDP vars and matrix
a = matrix.yalmipVars();
M = matrix.yalmipRealMatrix(a);

% Constraints (normalization, positivity)
constraints = [a(1) == 1];
constraints = [constraints, M>=0];

% Objective function (maximize)
objective = -CHSH_ineq.yalmip(a);

% Solve
optimize(constraints, objective); 

% Get solutions
solved_setting = chsh.Solved(a);
disp(struct2table(solved_setting.SymbolTable));

solved_FC = solved_setting.FullCorrelator;
disp(solved_FC.Values);

chsh_max_val = solved_setting.Value(CHSH_ineq)
