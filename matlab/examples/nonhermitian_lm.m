%% Clear
clear
clear mtk;

%% Params
mm_level = 1;
lm_level = max(mm_level - 1, 0);

%% Create setting
setting = AlgebraicScenario(["x1", "x2"], {{[1, 1], [1]}}, true);
setting.Complete(4);

[x1, x2] = setting.getAll();
re_x1x2 = (x1*x2 + x2*x1)*0.5;
im_x1x2 = (x1*x2 - x2*x1)*(-0.5i);

%% Make matrices 
mm = setting.MakeMomentMatrix(mm_level);
lm1 = re_x1x2.LocalizingMatrix(lm_level);
lm2 = im_x1x2.LocalizingMatrix(lm_level);

%% Define and solve SDP
cvx_begin sdp

    % Declare basis variables a (real) b (imaginary)
    mm.cvxVars('a', 'b');
    
    % Compose moment matrix from these basis variables
    M = mm.cvxComplexMatrix(a, b);
    
    % Compose localizing matrix from these basis variables
    L1 = lm1.cvxComplexMatrix(a, b);
    
    % Compose localizing matrix from these basis variables
    L2 = lm2.cvxComplexMatrix(a, b);
    
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