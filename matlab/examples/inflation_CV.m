clear
clear npatk

inflation_level = 2;
moment_matrix_level = 1;

pair = InflationScenario(inflation_level, ...
                             [0, 0], ...
                             {[1, 2]});

moment_matrix = pair.MakeMomentMatrix(moment_matrix_level);
disp(moment_matrix.SequenceMatrix);
disp(struct2table(pair.System.SymbolTable));
