 function loadSymbolInfoOrError(obj)
    % Cached value?
    if any(obj.symbol_id == -1, 'all')
        obj.loadSymbolInfo();

        if any(obj.symbol_id == -1, 'all')
            error("Some symbols were not found in matrix system.");
        end
    end
end