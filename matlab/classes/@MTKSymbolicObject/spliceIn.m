function mode = spliceIn(obj, indices, value)
    mode = spliceIn@MTKObject(obj, indices, value);
  
    switch mode
        case 0
            so_splice_scalar(obj, indices, value);
        case 1
            so_splice_array(obj, indices, value);
        case 2
            so_splice_array(obj, indices, value);
        case 3
            so_splice_broadcast(obj, indices, value);
    end    
end

%% Private functions
function obj = so_splice_scalar(obj, indices, value)
    if obj.IsScalar
        obj.SymbolCell = value.SymbolCell;
    else
        obj.SymbolCell{indices{:}} = value.SymbolCell{1};
    end
end

function obj = so_splice_array(obj, indices, value)
    [obj.SymbolCell{indices{:}}] = deal(value.SymbolCell{:});
end

function obj = so_splice_broadcast(obj, indices, value)
    [obj.SymbolCell{indices{:}}] = deal(value.SymbolCell{1});
end