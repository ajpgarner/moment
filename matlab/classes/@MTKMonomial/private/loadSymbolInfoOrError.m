 function loadSymbolInfoOrError(obj)
    % Cached value?
    if any(obj.symbol_id == -1)
        obj.loadSymbolInfo();

        if any(obj.symbol_id == -1)
            error("Some symbols were not found in matrix system.");
        end
    end
end