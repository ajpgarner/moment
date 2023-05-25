%%% Example inflation triangle, using distributions from arXiv:2203.16543
%%%
clear
clear mtk

%% Define triangle scenario and make moment matrix
inflation_level = 2;
moment_matrix_level = 2;
triangle = InflationScenario(inflation_level, ...
                             [4, 4, 4], ...
                             {[1, 2], [2, 3], [1, 3]});
 
moment_matrix = triangle.MakeMomentMatrix(moment_matrix_level);
extended_matrix = triangle.MakeExtendedMomentMatrix(moment_matrix_level);

%% Apply values
distribution = make_distribution(0.9);
symbol_assignments = triangle.GetAssignments(distribution);
substitutions = MomentRuleBook.ScalarSubstitutionCell(triangle, symbol_assignments);
subbed_matrix = substitutions.Apply(extended_matrix);

%% Define and solve SDP
yalmip('clear');

% Declare basis variables a (real)
a = subbed_matrix.yalmipVars;

% Compose moment matrix from these basis variables
M = subbed_matrix.yalmipRealMatrix(a);

% Constraints
constraints = [a(1) == 1, M >= 0];

% Do solve
diagnostics = optimize(constraints);
if diagnostics.problem == 0
 	feasible = true;
elseif diagnostics.problem == 1
    feasible = false;
else
    disp(diagnostics);
    error("Could not ascertain feasibility.");
end


%% Make the probability distribution according to arXiv:2203.16543 eq. 2
function dist = make_distribution(u) 
    % Index mapping; 0 -> 1, 1- -> 2, 1+ -> 3, 2 -> 4
    % also, note col-major ordering
    v = sqrt(1 - u*u);
    P = zeros(4, 4, 4);
        
    P(4,2,1) = u*u/8;
    P(1,4,2) = u*u/8;
    P(2,1,4) = u*u/8;
    
    P(3,4,1) = u*u/8;    
    P(1,3,4) = u*u/8;
    P(4,1,3) = u*u/8;
       
    P(4,3,1) = v*v/8;
    P(1,4,3) = v*v/8;
    P(3,1,4) = v*v/8;
    
    P(2,4,1) = v*v/8;
    P(1,2,4) = v*v/8;
    P(4,1,2) = v*v/8;
    
    P(2,2,2) = ((v*v*v - u*u*u)^2)/8;
    P(3,3,3) = ((v*v*v + u*u*u)^2)/8;
    
    P(3,2,2) = u*u*v*v*((u+v)^2)/8;
    P(2,3,2) = u*u*v*v*((u+v)^2)/8;
    P(2,2,3) = u*u*v*v*((u+v)^2)/8;
    
    P(2,3,3) = u*u*v*v*((u-v)^2)/8;
    P(3,2,3) = u*u*v*v*((u-v)^2)/8;
    P(3,3,2) = u*u*v*v*((u-v)^2)/8;
    
    dist = reshape(P, [1, 64]);   
end