function indices = cleanIndices(obj, indices)
% CLEANINDICES Replace ':' with 1:end as necessary
    if numel(indices)==1
        if ischar(indices{1}) && indices{1} == ':'
            indices{1} = 1:numel(obj);
        end
    else
        dims = size(obj);
        for idx = 1:numel(indices)
            if ischar(indices{idx}) && indices{idx} == ':'
                indices{idx} = 1:dims(idx);
            end
        end
    end
end