function mergeIn(obj, merge_dim, offsets, objects)

    merge_type = mergeIn@MTKObject(obj, merge_dim, offsets, objects);

    assert(~obj.IsScalar || numel(objects)==1);
    if obj.IsScalar
        assert(numel(objects)==1);
        obj.Constituents = objects{1}.Constituents; 
    else    
        m_constituents = (cellfun(@wrap_const_if_scalar, objects, ...
                              'UniformOutput', false));
        obj.Constituents = cat(merge_dim, m_constituents{:});     
    end      
end 

%% Private functions
function val = wrap_const_if_scalar(input)
    if iscell(input.Constituents)
        val = input.Constituents;
    else
        val = {input.Constituents};
    end
end