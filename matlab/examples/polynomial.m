addpath('..')
clear
clear npatk;

setting = AlgebraicScenario(2, {{[1, 1], [1]}}, true);
setting.Complete(20, true);

moment_matrix1 = setting.MakeMomentMatrix(1);
disp(moment_matrix1.SequenceMatrix);

x1x2 = setting.get([1 2])
x2x1 = setting.get([2 1])