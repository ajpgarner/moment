clear
clear mtk

inflation_level = 2;
moment_matrix_level = 2;
triangle = InflationScenario(inflation_level, ...
                             [2, 2, 2], ...
                             {[1, 2], [2, 3], [1, 3]});

moment_matrix = triangle.MakeMomentMatrix(moment_matrix_level);
disp(struct2table(triangle.System.SymbolTable));
disp(moment_matrix.SequenceMatrix);

% Impose P(000) = P(111) = 1/2
primal_symbols = triangle.ObservablesToSymbols(...
        {[1], [2], [3], [1 2], [1 3], [2 3], [1 2 3]});
subbed_matrix = moment_matrix.ApplyValues(primal_symbols, ...
    [0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5]);
disp(subbed_matrix.SequenceMatrix);

% Solve
cvx_begin sdp
    subbed_matrix.cvxVars('a')
    
    M = subbed_matrix.cvxRealMatrix(a);
    
    a(1) == 1;
    M >= 0;
    
cvx_end
feasible = ~strcmp(cvx_status, 'Infeasible');