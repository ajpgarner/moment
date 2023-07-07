function mode = spliceIn(obj, indices, value)
    mode = spliceIn@MTKObject(obj, indices, value);
  
    switch mode
        case 0
            poly_splice_scalar(obj, indices, value);
        case 1
            poly_splice_array(obj, indices, value);
        case 2
            poly_splice_array(obj, indices, value);
        case 3
            poly_splice_broadcast(obj, indices, value);
    end    
end

%% Private functions
function obj = poly_splice_scalar(obj, indices, value)
    if obj.IsScalar
        obj.Constituents = value.Constituents;
    else
        obj.Constituents{indices{:}} = value.Constituents;
    end
end

function obj = poly_splice_array(obj, indices, value)
    [obj.Constituents{indices{:}}] = deal(value.Constituents{:});
end

function obj = poly_splice_broadcast(obj, indices, value)
    [obj.Constituents{indices{:}}] = deal(value.Constituents);
end