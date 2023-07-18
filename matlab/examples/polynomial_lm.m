%% Example: polynomial_lm.m
% Demonstrates usage of polynomial localizing matrices.
% Analytical result: 1
%

%% Params
mm_level = 2;
lm_level = max(mm_level - 1, 0);

%% Create setting
setting = AlgebraicScenario(["x1", "x2"], ...
                            'rules', {{[1, 1], [1]}}, ...
                            'hermitian', true);

%% Create polynomials
[x1, x2] = setting.getAll();
re_x1x2 = (x1*x2 + x2*x1)*0.5;
im_x1x2 = (x1*x2 - x2*x1)*(-0.5i);

%% Make operator matrices 
mm = setting.MomentMatrix(mm_level);
lm1 = re_x1x2.LocalizingMatrix(lm_level);
lm2 = im_x1x2.LocalizingMatrix(lm_level);

%% Define and solve SDP
cvx_begin sdp

    % Declare basis variables a (real) b (imaginary)
    setting.cvxVars('a', 'b');
    
    % Compose moment matrix from these basis variables
    M = mm.cvx(a, b);
    
    % Compose localizing matrix from these basis variables
    L1 = lm1.cvx(a, b);
    
    % Compose localizing matrix from these basis variables
    L2 = lm2.cvx(a, b);
    
    % Constraints
    a(1) == 1;
    M >= 0;
    L1 >= 0;
    L2 >= 0;
    
    % Objective
    obj = x1.cvx(a, b);
    maximize(obj);
cvx_end

%% Display outcomes
format long
disp(obj)