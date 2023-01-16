clear
clear mtk

tic
inflation_level = 2;
moment_matrix_level = 1;
triangle = InflationScenario(inflation_level, ...
                             [2, 2, 2], ...
                             {[1, 2], [2, 3], [1, 3]});

moment_matrix = triangle.MakeMomentMatrix(moment_matrix_level);
disp(struct2table(triangle.System.SymbolTable));
disp(moment_matrix.SequenceMatrix);

subbed_matrix = moment_matrix.ApplyValues({{2, 0.5}, {3, 0.5}});
disp(subbed_matrix.SequenceMatrix);