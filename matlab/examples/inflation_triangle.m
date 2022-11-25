clear
clear npatk

inflation_level = 2;
moment_matrix_level = 1;

triangle = InflationScenario(inflation_level, ...
                             [2, 2, 2], ...
                             {[1, 2], [2, 3], [1, 3]});

moment_matrix = triangle.MakeMomentMatrix(moment_matrix_level);
disp(moment_matrix.SequenceMatrix);
disp(struct2table(triangle.System.SymbolTable));

%% Define and solve SDP
cvx_begin sdp

    % Declare basis variables a (real)
    moment_matrix.cvxVars('a');
    
    % Compose moment matrix from these basis variables
    M = moment_matrix.cvxRealMatrix(a);
    
    % Normalize, and PSD
    a(1) == 1;
    M >= 0;
    
cvx_end
    
    