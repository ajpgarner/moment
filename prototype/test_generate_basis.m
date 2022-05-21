ti_dense = [[1, 2, 3]; [2, 4, -3]; [3, -3, 5]];
ti_sparse = sparse(ti_dense );

dense_basis_sym_den = npatk('generate_basis', 'debug', 'symmetric', 'dense', ti_dense );
dense_basis_sym_spa = npatk('generate_basis', 'debug', 'symmetric', 'dense', ti_sparse);
[dense_basis_her_den_r, dense_basis_her_den_i] = npatk('generate_basis', 'debug', 'hermitian', 'dense', ti_dense );
[dense_basis_her_spa_r, dense_basis_her_spa_i] = npatk('generate_basis', 'debug', 'hermitian', 'dense', ti_sparse);

sparse_basis_sym_den = npatk('generate_basis', 'debug', 'symmetric', 'sparse', ti_dense );
sparse_basis_sym_spa = npatk('generate_basis', 'debug', 'symmetric', 'sparse', ti_sparse);
[sparse_basis_her_den_r, sparse_basis_her_den_i] = npatk('generate_basis', 'debug', 'hermitian', 'sparse', ti_dense );
[sparse_basis_her_spa_r, sparse_basis_her_spa_i, sparse_basis_her_spa_key] = npatk('generate_basis', 'debug', 'hermitian', 'sparse', ti_sparse);

clear npatk