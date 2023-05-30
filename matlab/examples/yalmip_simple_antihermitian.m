%% Clear
clear
clear mtk;

%% Params
mm_level = 1;

%% Create setting: X^2 = Y^2 = 1
setting = AlgebraicScenario(["X", "Y"], {{[1, 1], []}, {[2, 2], []}}, true);
setting.Complete(4);

[X, Y] = setting.getAll;

%% Make matrices and objective
mm = setting.MakeMomentMatrix(mm_level);
objective = 1i*(X*Y - Y*X);

%% Define and solve SDP via YALMIP
yalmip('clear');

    % Declare basis variables a (real)
    [a, b] = mm.yalmipVars;
    
    % Compose moment matrix from these basis variables
    M = mm.yalmipComplexMatrix(a, b);
    
    % Constraints
    constraints = [a(1) == 1, M >= 0];
    
    % Objective
    obj = objective.yalmip(a, b);    
    opt_results = optimize(constraints,obj);

%% Output solution
format long
value(obj)