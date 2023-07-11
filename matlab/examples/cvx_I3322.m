% EXAMPLE: CHSH scenario, with CVX
%
addpath('..')
clear
clear mtk;

%set level of the hierarchy
level = 4;

% Two parties
i3322 = LocalityScenario(2);
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
matrix = i3322.MomentMatrix(level);

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

% Make I3322 object manually
I3322_ineq = Corr12 + Corr21 ...
    - Corr20 - Corr11 - Corr02 - Corr01 - Corr10 - Corr00 ...
    - A1 - A0 - B1 - B0;

% Make I3322 object using FC tensor
I3322_ineq2 = i3322.FCTensor([[0 -1 -1  0]
                              [-1 -1 -1 -1]
                              [-1 -1 -1  1]
                              [0  -1  1  0]]);

        
% Define and solve SDP
cvx_begin sdp 
    % Declare basis variables a (real) and b (imaginary)
    matrix.cvxVars('a');
    
    % Compose moment matrix from these basis variables
    M = matrix.cvxRealMatrix(a);
     
    % Normalization
    a(1) == 1;
    
    % Positivity 
    M >= 0;
             
    % CHSH inequality (maximize!)
    i3322_ineq = I3322_ineq2.cvx(a);
    maximize(i3322_ineq);
cvx_end

% Get solutions
solved_setting = i3322.Solved(a);
disp(struct2table(solved_setting.SymbolTable));

format long
i3322_max_val = solved_setting.Value(I3322_ineq2)
fc_mat = solved_setting.FullCorrelator.Values
