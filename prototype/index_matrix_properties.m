function [dim, unique_count] = index_matrix_properties(indices)
%INDEX_MATRIX_PROPERTIES Get dimension and unique element count.
%   Raises assertion if index matrix is poorly defined.
    assert(isinteger(indices), "Input must be matrix of integers.")
    
    dim_sq = size(indices);
    assert(dim_sq(1)==dim_sq(2), "Input must be square matrix.");
    dim = dim_sq(1);
    
    assert(all(transpose(indices)==indices,'all'), ...
        "Input must be symmetric matrix.");
                
    unique_indices = unique(indices, 'stable');
    unique_count = numel(unique_indices);
    
    assert(1==unique_indices(1), "Indices must be greater than 1.");
    assert(unique_count==max(unique_indices), ...
        "Indices must be consecutive integers.");
     
end

