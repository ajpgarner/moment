function [objective] = tsirelson(M,desc,level)
%if all parties have the same number of measurements with the same number of outcomes
%desc is a vector of the form [parties, measurements, outcomes]
%if each party's measurements have the same number of outcomes
%desc is a vector of the form [a_outcomes, b_outcomes, ..., a_settings, b_settings, ...]
%otherwise desc is a cell where desc{i} is a vector with the number of outcomes of each
%of party i's measurements

if isa(desc,'double') && length(desc) == 3
	scenario = LocalityScenario(desc(1),desc(2),desc(3));
else
	scenario = LocalityScenario(desc);
end
	


% Make moment matrix
matrix = scenario.MakeMomentMatrix(level);

% Get SDP vars and matrix
a = matrix.yalmipVars();
Gamma = matrix.yalmipRealMatrix(a);
bell_functional = scenario.CGTensor(M);

% Constraints (normalization, positivity)
constraints = [a(1) == 1, Gamma >= 0];

% Objective function (maximize)
objective = bell_functional.yalmip(a);

% Solve
optimize(constraints, -objective); 
objective = value(objective);


end
