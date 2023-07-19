 function success = loadSymbolInfo(obj)
    % Set to default (and flag failure) if no matrix system yet.
    if ~obj.Scenario.HasMatrixSystem
         obj.setDefaultSymbolInfo();
         success = false;
         return;
    end
    
    % Call virtual method
    [obj.symbol_id, obj.symbol_conjugated, ...
     obj.re_basis_index, obj.im_basis_index, ...
     obj.is_alias] = obj.queryForSymbolInfo();
    
    % Success, if all symbols now identified.
    success = all(obj.symbol_id >= 0, 'all');
 end