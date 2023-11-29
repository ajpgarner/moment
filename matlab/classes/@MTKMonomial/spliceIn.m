function mode = spliceIn(obj, indices, value)
    mode = spliceIn@MTKObject(obj, indices, value);

    switch mode
        case 0
            if obj.IsScalar
                mono_assign_scalar(obj, value);
            else
                mono_splice_scalar(obj, indices, value);
            end
        case 1
            mono_splice_array(obj, indices, value);
        case 2
            mono_splice_array(obj, indices, value);
        case 3
            mono_splice_broadcast(obj, indices, value);
    end    
end

%% Private functions
function obj = mono_assign_scalar(obj, value)
    % Special case scalar assignment operator
    obj.Operators = value.Operators;   
    obj.Coefficient = value.Coefficient;
    obj.Hash = value.Hash;
    if value.symbol_id > 0 && obj.symbol_id >= 0
         obj.symbol_id = value.symbol_id;
         obj.symbol_conjugated = value.symbol_conjugated;
         obj.real_basis_index = value.real_basis_index;
         obj.im_basis_index = value.im_basis_index;
         obj.is_alias = value.is_alias;
    else
        obj.setDefaultSymbolInfo();       
    end        
end
    
function obj = mono_splice_scalar(obj, indices, value)    
    obj.Operators{indices{:}} = value.Operators;   
    obj.Coefficient(indices{:}) = value.Coefficient;
    obj.Hash(indices{:}) = value.Hash;
    
    if value.symbol_id > 0 && all(obj.symbol_id(:) >= 0)
         obj.symbol_id(indices{:}) = value.symbol_id;
         obj.symbol_conjugated(indices{:}) = value.symbol_conjugated;
         obj.re_basis_index(indices{:}) = value.re_basis_index;
         obj.im_basis_index(indices{:}) = value.im_basis_index;
         obj.is_alias(indices{:}) = value.is_alias;
    else
        obj.setDefaultSymbolInfo();       
    end
end

function obj = mono_splice_array(obj, indices, value)
    [obj.Operators{indices{:}}] = deal(value.Operators{:});
    obj.Coefficient(indices{:}) = value.Coefficient(:);
    obj.Hash(indices{:}) = value.Hash(:);
    
    if value.symbol_id > 0 && all(obj.symbol_id(:) >= 0)
         obj.symbol_id(indices{:}) = value.symbol_id(:);
         obj.symbol_conjugated(indices{:}) = value.symbol_conjugated(:);
         obj.re_basis_index(indices{:}) = value.re_basis_index(:);
         obj.im_basis_index(indices{:}) = value.im_basis_index(:);
         obj.is_alias(indices{:}) = value.is_alias(:);
    else
        obj.setDefaultSymbolInfo();
    end
end


function obj = mono_splice_broadcast(obj, indices, value)
    [obj.Operators{indices{:}}] = deal(value.Operators);
    obj.Coefficient(indices{:}) = value.Coefficient;
    obj.Hash(indices{:}) = value.Hash;
    
    if value.symbol_id > 0 && all(obj.symbol_id(:) >= 0)
         obj.symbol_id(indices{:}) = value.symbol_id;
         obj.symbol_conjugated(indices{:}) = value.symbol_conjugated;
         obj.re_basis_index(indices{:}) = value.re_basis_index;
         obj.im_basis_index(indices{:}) = value.im_basis_index;
         obj.is_alias(indices{:}) = value.is_alias;
    else
        obj.setDefaultSymbolInfo();
    end
end
