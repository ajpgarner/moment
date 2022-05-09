fprintf('DENSE:\n');
input_matrix = [[1, 1, 3]; [2, 2, 4]; [2, 4, 5]];
constraints = impose_symmetric_constraints(input_matrix)

% fprintf('\nSPARSE:\n');
% im_sparse = sparse(input_matrix);
% constraints2 = impose_symmetric_constraints(im_sparse)

% filename = 'test_data/2moment1';
% raw_big_sparse_matrix = load(filename);
% big_sparse_matrix = spconvert(raw_big_sparse_matrix);
%constraints3 = impose_symmetric_constraints(big_sparse_matrix);

clear impose_symmetric_constraints