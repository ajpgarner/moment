function mergeIn(obj, merge_dim, offsets, objects)

    merge_type = mergeIn@MTKObject(obj, merge_dim, offsets, objects);

    % If scalar, promote constituent list to cell before merge.
    switch merge_type
        case {0, 1} % Scalar to row/col vec                               
            m_constituents = (cellfun(@(x) {x.Constituents}, ...
                              objects, 'UniformOutput', false));
        % FIXME: Row vector to larger-row-vec
        otherwise
            m_constituents = (cellfun(@(x) x.Constituents, ...
                           objects, 'UniformOutput', false));               
    end

    obj.Constituents = cat(merge_dim, m_constituents{:});                  
end  