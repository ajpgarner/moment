addpath('..')
clear
clear npatk;

setting = AlgebraicScenario(2, {{[1, 1], [1]}}, true);
setting.Complete(20, true);

moment_matrix1 = setting.MakeMomentMatrix(1);
moment_matrix2 = setting.MakeMomentMatrix(2);