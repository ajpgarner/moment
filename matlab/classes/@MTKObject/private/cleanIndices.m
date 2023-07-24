function indices = cleanIndices(obj, indices)
% CLEANINDICES Replace ':' with 1:end as necessary

    % Check indices are in bounds w.r.t. object size
    dims = size(obj);
    if numel(indices) > numel(dims)
        error("Cannot provide more indices than the dimension of the object.");
    end
    
    % Replace logical indices with numeric indices
    for dIdx = 1:numel(indices)
        if islogical(indices{dIdx})
            indices{dIdx} = find(indices{dIdx});
        end
    end
    
    % Replace ':' with full range
    if numel(indices)==1
        last_limit = numel(obj);
        if ischar(indices{1}) && indices{1} == ':'
            indices{1} = 1:last_limit;
        elseif any(indices{1} > last_limit)
                bad = indices{1}(find(indices{1} > last_limit, 1));
                error("Index '%d' is out of bounds.", bad);
        end
    else
        for idx = 1:(numel(indices)-1)
            if ischar(indices{idx}) && indices{idx} == ':'
                indices{idx} = 1:dims(idx);
            else 
                if any(indices{idx} > dims(idx))
                    bad = indices{idx}(find(indices{idx} > dims(idx), 1));
                    error("Index '%d' in dimension %d is out of bounds.",...
                          bad, idx);
                end
            end
        end
        
        % Last element ':' goes over all remaining dimensions
        last_limit = prod(dims(numel(indices):end));
        if ischar(indices{end}) && indices{end} == ':'
            indices{end} = 1:last_limit;
        elseif any(indices{end} > last_limit)
                bad = indices{end}(find(indices{end} > last_limit, 1));
                error("Index '%d' in dimension %d is out of bounds.",...
                      bad, numel(indices));
        end
    end   
end