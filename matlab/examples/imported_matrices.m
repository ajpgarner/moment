%% Example: imported_matrices.m
% Shows how a matrix of symbol names can be imported into Moment, and how
% the automatic Hermitian detection works.
%

imported_scenario = ImportedScenario();

test_input = [["1", "0.5#2", "3"]; ["0.5#2*", "4", "3"]; ["3*", "3*", "5"]];

imported_matrix = imported_scenario.ImportHermitianMatrix(test_input);
disp(imported_matrix.SymbolStrings)
disp(imported_matrix.SequenceStrings)
disp(imported_scenario.Symbols)

% Since we import as Hermitian, this infers 2 = 2* and makes 2 Hermitian.
test_input_2 = [[1, 2]; [2, 6]];

imported_matrix_2 = imported_scenario.ImportHermitianMatrix(test_input_2);
disp(imported_matrix_2.SymbolStrings)
disp(imported_matrix_2.SequenceStrings)
disp(imported_scenario.Symbols)
