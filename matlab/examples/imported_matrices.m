clear
clear mtk

ref_id = mtk('new_imported_matrix_system');

test_input = [[1, 2, 3]; [2, 4, 3]; [3, 3, 5]];

matrix_index = mtk('import_matrix', ref_id, test_input);
test_output = mtk('operator_matrix', 'symbols', ref_id, matrix_index);
disp(test_output)
