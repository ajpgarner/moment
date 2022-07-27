clear
clear npatk;

% Two parties
chsh = Setting(2);
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

% Make CHSH object
CHSH_ineq = Corr00 + Corr01 + Corr10 - Corr11;

% Define and solve SDP
cvx_begin sdp quiet
     [a, b, M] = matrix.cvxHermitianBasis();
     
     % Normalization
     a(1) == 1;
     
     % Positivity 
     M >= 0;
             
     % CHSH inequality (maximize!)
     chsh_ineq = CHSH_ineq.cvx(a);     
     maximize(chsh_ineq);
cvx_end

% Get solutions
solved_matrix = SolvedMomentMatrix(chsh, matrix, a, b);
disp(struct2table(solved_matrix.SymbolTable))
solved_setting = solved_matrix.Setting;
