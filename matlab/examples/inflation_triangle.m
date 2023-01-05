clear
clear mtk

tic
inflation_level = 2;
moment_matrix_level = 4;
triangle = InflationScenario(inflation_level, ...
                             [2, 2, 2], ...
                             {[1, 2], [2, 3], [1, 3]});

moment_matrix = triangle.MakeMomentMatrix(moment_matrix_level);
toc