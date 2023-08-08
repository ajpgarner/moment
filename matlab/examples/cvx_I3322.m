%% EXAMPLE: cvx_I3322
% Evaluate the I3322 CHSH inequality with CVX
%

% Set level of the hierarchy
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


% Make I3322 object using FC tensor
I3322_ineq = i3322.FCTensor([[0 -1 -1  0]
                             [-1 -1 -1 -1]
                             [-1 -1 -1  1]
                             [0  -1  1  0]]);

        
% Define and solve SDP
cvx_begin sdp 
    % Declare basis variables a (real) and b (imaginary)
    i3322.cvxVars('a');
    
    % Compose moment matrix from these basis variables
    M = matrix.cvx(a);
     
    % Normalization
    a(1) == 1;
    
    % Positivity 
    M >= 0;
             
    % CHSH inequality (maximize!)
    i3322_ineq = I3322_ineq.cvx(a);
    maximize(i3322_ineq);
cvx_end

% Get solution
i3322_max_val = I3322_ineq.Apply(a)