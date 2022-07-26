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

% Define and solve SDP
cvx_begin sdp
     [a, b, M] = matrix.cvxHermitianBasis();
     
     % Normalization
     a(1) == 1;
     
     % Positivity 
     M >= 0;
     
     % Correlations
     corr00 = Corr00.cvx(a);
     corr01 = Corr01.cvx(a);
     corr10 = Corr10.cvx(a);
     corr11 = Corr11.cvx(a);
             
     % CHSH inequality
     expression chsh_ineq;
     chsh_ineq = corr00 + corr01 + corr10 - corr11;        
     maximize(chsh_ineq);
cvx_end

solved_matrix = SolvedMomentMatrix(matrix, a, b);
disp(struct2table(solved_matrix.SymbolTable))
