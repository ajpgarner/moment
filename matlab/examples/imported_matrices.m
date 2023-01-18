clear
clear mtk

ref_id = mtk('new_imported_matrix_system', 'real');

test_input = [[1, 2, 3]; [2, 4, 3]; [3, 3, 5]];

matrix_index = mtk('import_matrix', ref_id, test_input);
parsed_symbols = mtk('operator_matrix', 'symbols', ref_id, matrix_index);
parsed_sequences = mtk('operator_matrix','sequences',ref_id, matrix_index);

disp(parsed_symbols);
disp(parsed_sequences);
disp(struct2table(mtk('symbol_table', ref_id)))

[b_re, b_im] = mtk('generate_basis', ref_id, matrix_index, 'dense', 'cell');