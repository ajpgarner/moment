%% Example: yalmip_inflation_triangle.m
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
                         
%% Generate moment matrix
moment_matrix = triangle.MomentMatrix(moment_matrix_level);

%% Impose P(000) = P(111) = 1/2
[A, B, C] = triangle.getPrimaryVariants();
ABC = A * B * C;
distribution = ABC.Probability([0.5, 0, 0, 0, 0, 0, 0, 0.5]);

substitutions = triangle.MomentRulebook();
substitutions.Add(distribution);
    
subbed_matrix = substitutions.Apply(moment_matrix);

%% Solve via Yalmip
yalmip('clear');

% Declare basis variables a (real)
a = triangle.yalmipVars;

% Compose moment matrix from these basis variables
M = subbed_matrix.Apply(a);

% Constraints
constraints = [a(1) == 1, M >= 0];

% Do solve
diagnostics = optimize(constraints);

%% Print result
if diagnostics.problem == 0
 	feasible = true;
    fprintf("Feasible (incorrect answer).\n");
elseif diagnostics.problem == 1
    feasible = false;
    fprintf("Infeasible (correct answer).\n");
else
    disp(diagnostics);
    error("Could not ascertain feasibility.");
end
