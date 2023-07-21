%% Example: yalmip_polynomial.m
% Solves example in https://doi.org/10.1137/090760155
% Expected answer: -0.75
%

%% Params
mm_level = 3;
lm_level = max(mm_level - 1, 0);

%% Create setting
setting = AlgebraicScenario(2, ...
            'rules', {{[1, 1], [1]}}, ...
            'hermitian', true);
setting.Complete(4);

[x1, x2] = setting.getAll();
I = setting.id(); 

objective = x1 * x2 + x2 * x1;
poly = -x2 * x2 + x2 + 0.5*I;

%% Make matrices 
mm = setting.MomentMatrix(mm_level);
lm = poly.LocalizingMatrix(lm_level);

%% Define and solve SDP via YALMIP
yalmip('clear');

    % Declare basis variables a (real)
    a = setting.yalmipVars;
    
    % Compose moment matrix from these basis variables
    M = mm.yalmip(a);
    
    % Compose localizing matrix from these basis variables
    L = lm.yalmip(a);
    
    % Constraints
    constraints = [a(1) == 1, M >= 0, L >= 0];
    
    % Objective
    obj = objective.yalmip(a);    
    opt_results = optimize(constraints,obj);

%% Output solution
format long
obj = value(obj)
