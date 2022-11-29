%% Clear
addpath('..')
clear
clear npatk;

%% Params
mm_level = 4;
lm_level = max(mm_level - 1, 0);

%% Create setting
setting = AlgebraicScenario(2, {{[1, 1], [1]}}, true);
setting.Complete(4);

x1x2 = setting.get([1 2]);
x2x1 = setting.get([2 1]);
x2x2 = setting.get([2 2]);
x2 = setting.get([2]);
I = setting.get([]);

objective = x1x2 + x2x1;
poly = -x2x2 + x2 + 0.5*I;

%% Make matrices 
mm = setting.MakeMomentMatrix(mm_level);
lm = poly.LocalizingMatrix(lm_level);

%% Define and solve SDP
cvx_begin sdp

    % Declare basis variables a (real)
    mm.cvxVars('a');
    
    % Compose moment matrix from these basis variables
    M = mm.cvxRealMatrix(a);
    
    % Compose localizing matrix from these basis variables
    L = lm.cvxRealMatrix(a);
    
    % Constraints
    a(1) == 1;
    M >= 0;
    L >= 0;
    
    % Objective
    obj = objective.cvx(a);    
    minimize(obj)
cvx_end

disp(M)