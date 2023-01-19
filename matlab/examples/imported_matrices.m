clear
clear mtk

imported_scenario = ImportedScenario();
ref_id = imported_scenario.System().RefId;

test_input = [[1, 2, 3]; [2, 4, 3]; [3, 3, 5]];

imported_matrix = imported_scenario.ImportHermitianMatrix(test_input);
disp(imported_matrix.SymbolMatrix)
disp(imported_matrix.SequenceMatrix)
disp(struct2table(imported_scenario.System.SymbolTable))

[b_re, b_im] = imported_matrix.DenseBasis();
