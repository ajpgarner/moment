addpath('..')
clear
clear npatk;

setting = AlgebraicScenario(2, {{[2, 1], '-', [1, 2]}}, true);
setting.Complete(20, true);

moment_matrix1 = setting.MakeMomentMatrix(1);
disp(moment_matrix1.SequenceMatrix);
moment_matrix2 = setting.MakeMomentMatrix(2);
disp(moment_matrix2.SequenceMatrix);

disp(struct2table(setting.System.SymbolTable));