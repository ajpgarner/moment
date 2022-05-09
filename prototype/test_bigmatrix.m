filename = 'test_data/2moment1';
raw_sparse_matrix = load(filename);
input_matrix = spconvert(raw_sparse_matrix);

[symmetrized_matrix, constraints] = symmetrize_index_matrix(input_matrix);


constraints

