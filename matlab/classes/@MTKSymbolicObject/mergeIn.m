function mergeIn(obj, merge_dim, offsets, objects)

    merge_type = mergeIn@MTKObject(obj, merge_dim, offsets, objects);

    % Merge cells
    symbol_cells = cellfun(@(x) x.SymbolCell, objects, ...
                           'UniformOutput', false);   
    obj.SymbolCell = cat(merge_dim, symbol_cells{:});
    
    % Check if merged cells have any polynomial objects
    obj.IsPolynomial = test_if_polynomial(obj);
   
end 
