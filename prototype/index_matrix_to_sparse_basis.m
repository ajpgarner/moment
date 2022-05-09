function [basis_list] = index_matrix_to_sparse_basis(indices)
%INDEX_MATRIX_TO_SPARSE_BASIS Convert matrix of indices to a sparse basis.
%   Raises assertion if index matrix is poorly defined.

    [dim, basis_size] = index_matrix_properties(indices);
    basis_list = repelem({sparse(dim,dim)}, 1, basis_size);
    
    for i = 1:dim
        for j = i:dim
            index = indices(i,j);
            basis_list{index}(i,j) = 1;
            basis_list{index}(j,i) = 1;
        end
    end
end
