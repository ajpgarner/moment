function [symmetrized, constraints, zero_indices] = symmetrize_index_matrix(input_matrix)
    
    % Get matrix dimensions, and check is a square matrix
    dim_sq = size(input_matrix);
    assert(dim_sq(1)==dim_sq(2), "Input must be square matrix.");
    dim = dim_sq(1);
    
    % Get largest referenced variable name
    [minIM,maxIM] = bounds(input_matrix,'all');
    if (minIM < 0) && (-minIM > maxIM)
        absMaxVN = full(-minIM);
    else
        absMaxVN = full(maxIM);
    end
    
    % Get list elements that are not symmetric already
    transposed_input = transpose(input_matrix);    
    non_matching = (triu(input_matrix) ~= triu(transposed_input));    
    upper_indices = full(input_matrix(non_matching));
    lower_indices = full(transposed_input(non_matching));
    assert(numel(upper_indices) == numel(lower_indices));
    constraint_types = zeros(numel(upper_indices), 1);
    
    % Flip elements, such that upper <= lower, and upper is always positive:
    for k = 1:numel(upper_indices)
        assert(upper_indices(k) ~= lower_indices(k));
        
        upper_varname = upper_indices(k);
        lower_varname = lower_indices(k);
        upper_neg = upper_varname < 0;
        lower_neg = lower_varname < 0;        
        negation = xor(upper_neg, lower_neg);
        
        if upper_neg
            upper_varname = -upper_varname;
        end
        if lower_neg
            lower_varname = -lower_varname;
        end
      
        if upper_varname > lower_varname
            upper_indices(k) = lower_varname;
            lower_indices(k) = upper_varname;
        else
            upper_indices(k) = upper_varname;
            lower_indices(k) = lower_varname;
        end 
        
        if negation
            constraint_types(k) = 0x02; % a == -b
        else
            constraint_types(k) = 0x01; % a == b
        end        
    end
    
    % Order lists, first by upper index; then by lower index
    constraints = unique([upper_indices, lower_indices, constraint_types], 'rows', 'sorted');
    zero_indices = [];
        
    symmetrized = [];
end
