 function [re, im] = calculateCoefficients(obj)
 
    % Early exit if we can't get symbol information for all parts...
    if obj.IsScalar                
        if ~all(obj.Constituents.FoundSymbol(:))
            error(obj.err_missing_symbol);
        end
    else
        found_symbols = cellfun(@(c) all(c.FoundSymbol(:)), obj.Constituents);
        if ~all(found_symbols(:))
            error(obj.err_missing_symbol);
        end
    end
    
    % Query MTK for basis from symbolic representation.   
	[re, im] = mtk('generate_basis', ...
				   obj.Scenario.System.RefId, obj.SymbolCell);

end
