% EXAMPLE: CHSH scenario, with CVX
%
addpath('..')
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
matrix = chsh.MakeMomentMatrix(2);

% Make correlator objects
Corr00 = Correlator(A0, B0);
Corr01 = Correlator(A0, B1);
Corr10 = Correlator(A1, B0);
Corr11 = Correlator(A1, B1);

% Make CHSH object manually
CHSH_ineq = Corr00 + Corr01 + Corr10 - Corr11;

% Alternatively, make via full-correlator
fc = FullCorrelator(chsh);
CHSH_ineq2 = fc.linfunc([[0 0 0]; [0 1 1]; [0 1 -1]]);

% Define and solve SDP
cvx_begin sdp quiet
     [a, b, M] = matrix.cvxHermitianBasis();
     
     % Normalization
     a(1) == 1;
     
     % Positivity 
     M >= 0;
             
     % CHSH inequality (maximize!)
     solve_chsh_ineq = CHSH_ineq.cvx(a);
     maximize(solve_chsh_ineq);
cvx_end

% Get solutions
solved_setting = SolvedScenario(chsh, matrix, a, b);
solved_matrix = solved_setting.SolvedMomentMatrix;
disp(struct2table(solved_matrix.SymbolTable));
chsh_max_val = solved_setting.Value(CHSH_ineq);
