%% Clear
addpath('..')
clear
clear npatk;

%% Params
mm_level = 1;
lm_level = max(mm_level - 1, 0);

%% Create setting
setting = AlgebraicScenario(2, {{[1, 1], [1]}}, true);
setting.Complete(5);

x1x2 = setting.get([1 2]);
x2x1 = setting.get([2 1]);
x2x2 = setting.get([2 2]);
x2 = setting.get([2]);
I = setting.get([]);

objective = x1x2 + x2x1;
poly = -x2x2 + x2 + 0.5*I;

%% Make matrices 
mm = setting.MakeMomentMatrix(mm_level);
lm_x2x2 = x2x2.LocalizingMatrix(lm_level);
lm_x2 = x2.LocalizingMatrix(lm_level);
lm_I = I.LocalizingMatrix(lm_level);