% EXAMPLE: CHSH scenario, with CVX
%
addpath('..')
clear
clear npatk;

% Two parties
scenario = LocalityScenario(2);
Alice = scenario.Parties(1);
Bob = scenario.Parties(2);

% Each party with two measurements
A0 = Alice.AddMeasurement(2);
A1 = Alice.AddMeasurement(2);
B0 = Bob.AddMeasurement(2);
B1 = Bob.AddMeasurement(2);

% Make moment matrix
matrix = scenario.MakeMomentMatrix(6);

% Make correlator objects
Corr00 = Correlator(A0, B0);
Corr01 = Correlator(A0, B1);
Corr10 = Correlator(A1, B0);
Corr11 = Correlator(A1, B1);

% Make CHSH object manually
CHSH_ineq = Corr00 + Corr01 + Corr10 - Corr11;

% Alternatively, make via full-correlator
CHSH_ineq2 = scenario.FCTensor([[0 0 0]; [0 1 1]; [0 1 -1]]);

% Alternatively, make via Collins-Gisin notation
CHSH_ineq3 = scenario.CGTensor([[2 -4 0]; [-4 4 4]; [0 4 -4]]);

% Define and solve SDP
cvx_begin sdp

    % Declare basis variables a (real) and b (imaginary)
    matrix.cvxVars('a', 'b');
    
    % Compose moment matrix from these basis variables
    M = matrix.cvxHermitianBasis(a, b);

    % Normalization
    a(1) == 1;

    % Positivity
    M >= 0;

    % CHSH inequality (maximize!)
    solve_chsh_ineq = CHSH_ineq.cvx(a);
    maximize(solve_chsh_ineq);
cvx_end

% Get solutions
solved_setting = SolvedScenario(scenario, matrix, a, b);
solved_matrix = solved_setting.SolvedMomentMatrix;
disp(struct2table(solved_matrix.SymbolTable));

% Print out values found (should be identical!)
chsh_max_val = solved_setting.Value(CHSH_ineq)
chsh_max_val2 = solved_setting.Value(CHSH_ineq2)
chsh_max_val3 = solved_setting.Value(CHSH_ineq3)
