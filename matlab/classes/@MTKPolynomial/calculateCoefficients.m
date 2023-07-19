 function [re, im] = calculateCoefficients(obj)
 
    % Early exit if we can't get symbol information for all parts...
    if obj.IsScalar                
        if ~all(obj.Constituents.FoundSymbol, 'all')
            error(obj.err_missing_symbol);
        end
    else
        if ~all(cellfun(@(c) all(c.FoundSymbol, 'all'), ...
                obj.Constituents), 'all')
            error(obj.err_missing_symbol);
        end
    end
    
    % Query MTK for basis from symbolic representation.
    if obj.IsScalar
        [re, im] = mtk('generate_basis', ...
                       obj.Scenario.System.RefId, {obj.SymbolCell});
    else
        [re, im] = mtk('generate_basis', ...
                       obj.Scenario.System.RefId, obj.SymbolCell);
    end
end