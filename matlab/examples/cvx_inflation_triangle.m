%% Example: cvx_inflation_triangle.m
% Test triangle scenario of three binary observables, each pair-wise linked 
% by a source, with total probability distribution P(000) = P(111) = 1/2.
%   
% Expected outcome: Infeasible
%

%% Construct scenario
inflation_level = 2;
moment_matrix_level = 2;
triangle = InflationScenario(inflation_level, ...
                             [2, 2, 2], ...
                             {[1, 2], [2, 3], [1, 3]});

moment_matrix = triangle.MomentMatrix(moment_matrix_level);
disp(struct2table(triangle.System.SymbolTable));
disp(moment_matrix.SequenceStrings);

%% Impose P(000) = P(111) = 1/2
[A, B, C] = triangle.getPrimaryVariants();
ABC = A * B * C;
sub_list = ABC.Probability([0.5, 0, 0, 0, 0, 0, 0, 0.5], 'list');
    
substitutions = triangle.MomentRulebook();
substitutions.AddFromList(sub_list);
    
subbed_matrix = substitutions.Apply(moment_matrix);
disp(subbed_matrix.SequenceStrings);

%% Solve via CVX
cvx_begin sdp
    triangle.cvxVars('a')
    
    M = subbed_matrix.cvx(a);
    
    a(1) == 1;
    M >= 0;
    
cvx_end
feasible = ~strcmp(cvx_status, 'Infeasible');
