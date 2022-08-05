% EXAMPLE: CHSH scenario, with CVX
%
addpath('..')
clear
clear npatk;

% Two parties
i3322 = Scenario(2);
Alice = i3322.Parties(1);
Bob = i3322.Parties(2);

% Each party with two measurements
A0 = Alice.AddMeasurement(2);
A1 = Alice.AddMeasurement(2);
A2 = Alice.AddMeasurement(2);
B0 = Bob.AddMeasurement(2);
B1 = Bob.AddMeasurement(2);
B2 = Bob.AddMeasurement(2);

% Make moment matrix
matrix = i3322.MakeMomentMatrix(4);

% Make correlator objects
Corr00 = Correlator(A0, B0);
Corr01 = Correlator(A0, B1);
Corr02 = Correlator(A0, B2);
Corr10 = Correlator(A1, B0);
Corr11 = Correlator(A1, B1);
Corr12 = Correlator(A1, B2);
Corr20 = Correlator(A2, B0);
Corr21 = Correlator(A2, B1);
Corr22 = Correlator(A2, B2);

% Make CHSH object
I3322_ineq = Corr12 + Corr21 - Corr20 - Corr11 - Corr02 - Corr01 - Corr10 - Corr00 ...
    - A0.Outcomes(1) - A1.Outcomes(1) - B0.Outcomes(1) - B1.Outcomes(1) ...
    + A0.Outcomes(2) + A1.Outcomes(2) + B0.Outcomes(2) + B1.Outcomes(2);
    % - A1 - A0 - B1 - B0;
    
% Define and solve SDP
cvx_begin sdp 
     [a, M] = matrix.cvxSymmetricBasis();
     
     % Normalization
     a(1) == 1;
    
     % Positivity 
     M >= 0;
     
             
     % CHSH inequality (maximize!)
     i3322_ineq = I3322_ineq.cvx(a);
     maximize(i3322_ineq);
cvx_end

% Get solutions
solved_setting = SolvedScenario(i3322, matrix, a);
solved_matrix = solved_setting.SolvedMomentMatrix;
disp(struct2table(solved_matrix.SymbolTable));
format long
i3322_max_val = solved_setting.Value(I3322_ineq)
