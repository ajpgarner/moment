clear
clear npatk

inflation_level = 2;
moment_matrix_level = 1;

triangle = InflationScenario(inflation_level, ...
                             [2, 2, 2], ...
                             {[1, 2], [2, 3], [1, 3]});

moment_matrix = triangle.MakeMomentMatrix(moment_matrix_level);
disp(moment_matrix.SequenceMatrix);
disp(struct2table(triangle.System.SymbolTable));
