% EXAMPLE: CHSH scenario, using yalmip
%
addpath('..')
yalmip('clear')
clear
clear npatk;

% Two parties
chsh = Scenario(2);
Alice = chsh.Parties(1);
Bob = chsh.Parties(2);

% Each party with two measurements
A0 = Alice.AddMeasurement(2);
A1 = Alice.AddMeasurement(2);
B0 = Bob.AddMeasurement(2);
B1 = Bob.AddMeasurement(2);

% Make moment matrix
matrix = chsh.MakeMomentMatrix(1);

% Make correlator objects
Corr00 = Correlator(A0, B0);
Corr01 = Correlator(A0, B1);
Corr10 = Correlator(A1, B0);
Corr11 = Correlator(A1, B1);

% Make CHSH object
CHSH_ineq = Corr00 + Corr01 + Corr10 - Corr11;

% Get SDP vars and matrix
[a, b] = matrix.yalmipVars();
M = matrix.yalmipHermitianBasis(a, b);

% Constraints (normalization, positivity)
constraints = [a(1) == 1];
constraints = [constraints, M>=0];

% Objective function (maximize)
objective = -CHSH_ineq.yalmip(a);

% Solve
optimize(constraints, objective); 

% Get solutions
solved_setting = SolvedScenario(chsh, matrix, a, b);
solved_matrix = solved_setting.SolvedMomentMatrix;
disp(struct2table(solved_matrix.SymbolTable));
chsh_max_val = solved_setting.Value(CHSH_ineq)
