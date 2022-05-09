function [sorted, key, nu_diagonal, nu_elems] = sort_index_matrix(indices)
%sort_index_matrix Re-number index matrix in canonical ordering
%   1 to nu_diagonal reflect unique diagonal elements, 
%   nu_diagonal+1 to nu_elems for rest of indices
%   
    assert(isinteger(indices), "Input must be matrix of integers.")
    
    dim_sq = size(indices);
    assert(dim_sq(1)==dim_sq(2), "Input must be square matrix.");
    dim = dim_sq(1);
    
    
    % Possibly move to C code:
    key = containers.Map('KeyType', 'uint64', 'ValueType', 'uint64');
    index = 1;
    
    sorted = zeros(dim,dim);
    
    % First, diagonals:
    for i = 1:dim
        if key.isKey(indices(i,i))
            sorted(i,i) = key(indices(i,i));
        else
            sorted(i,i) = index;
            key(indices(i,i)) = index;
            index = index + 1;
        end
    end
    nu_diagonal = index - 1;
        
    % Now, upper triangle
    for i = 1:dim
        for j = (i+1):dim
            if key.isKey(indices(i,j))
                sorted(i,j) = key(indices(i,j));
                sorted(j,i) = key(indices(i,j));
            else
                sorted(i,j) = index;
                sorted(j,i) = index;
                key(indices(i,j)) = index;
                index = index + 1;
            end
        end
    end
    
    nu_elems = index - 1;
  
end

