%% Example: inflation_CV.m
% Demonstrates algebraic manipulations in continuous variable scenario.
%

clear
clear mtk

inflation_level = 2;
moment_matrix_level = 1;

line = InflationScenario(inflation_level, ...
                             [0, 0, 0], ...
                             {[1, 2], [2, 3]});
moment_matrix = line.MomentMatrix(moment_matrix_level);
disp(moment_matrix.SequenceStrings);

[A0, A1, B00, B10, B01, B11, C0, C1] = line.getVariants;
