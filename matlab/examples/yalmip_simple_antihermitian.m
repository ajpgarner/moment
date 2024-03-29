%% Example: yalmip_simple_anti_hermitian.m
% Expected answer: -1
%

%% Params
mm_level = 1;

%% Create setting: X^2 = Y^2 = 1
setting = AlgebraicScenario(["X", "Y"], ...
        'rules', {{[1, 1], []}, {[2, 2], []}}, ...
        'hermitian', true);    
setting.Complete(4);

[X, Y] = setting.getAll;

%% Make matrices and objective
mm = setting.MomentMatrix(mm_level);
objective = -0.5i*(X*Y - Y*X);

%% Define and solve SDP via YALMIP
yalmip('clear');

    % Declare basis variables a (real)
    [a, b] = setting.yalmipVars;
    
    % Compose moment matrix from these basis variables
    M = mm.Apply(a, b);
    
    % Constraints
    constraints = [a(1) == 1, M >= 0];
    
    % Objective
    obj = objective.Apply(a, b);    
    opt_results = optimize(constraints, obj);

%% Output solution
format long
value(obj)
