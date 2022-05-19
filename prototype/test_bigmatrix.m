filename = 'test_data/2moment1';
raw_sparse_matrix = load(filename);
input_matrix = spconvert(raw_sparse_matrix);

symmetrized_matrix = npatk('make_symmetric', input_matrix);
clear npatk



