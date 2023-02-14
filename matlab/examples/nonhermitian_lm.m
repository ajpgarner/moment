%% Clear
addpath('..')
clear
clear mtk;

%% Params
mm_level = 4;
lm_level = max(mm_level - 1, 0);

%% Create setting
setting = AlgebraicScenario(2, {{[1, 1], [1]}}, true);
setting.Complete(4);

x1 = setting.get([1]);
x1x2 = setting.get([1 2]);
x2x1 = setting.get([2 1]);
x1x2_plus_x2x1 = x1x2 + x2x1;

%% Make matrices 
mm = setting.MakeMomentMatrix(mm_level);
lm = x1x2_plus_x2x1.LocalizingMatrix(lm_level);

%% Define and solve SDP
cvx_begin sdp

    % Declare basis variables a (real) b (imaginary)
    mm.cvxVars('a', 'b');
    
    % Compose moment matrix from these basis variables
    M = mm.cvxComplexMatrix(a, b);
    
    % Compose localizing matrix from these basis variables
    L = lm.cvxComplexMatrix(a, b);
    
    % Constraints
    a(1) == 1;
    M >= 0;
    L >= 0;
    
    % Objective
    obj = x1.cvx(a);    
    maximize(obj)
cvx_end

%% Display outcomes
format long
disp(obj)