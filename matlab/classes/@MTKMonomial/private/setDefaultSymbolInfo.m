function setDefaultSymbolInfo(obj)
    [obj.symbol_id, obj.symbol_conjugated, ...
     obj.re_basis_index, obj.im_basis_index, ...
     obj.is_alias] = getDefaultSymbolInfo(obj);    
end