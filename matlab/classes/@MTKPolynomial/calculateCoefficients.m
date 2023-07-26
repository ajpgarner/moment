 function [re, im] = calculateCoefficients(obj)
 
    % Early exit if we can't get symbol information for all parts...
    if ~checkAllSymbolsFound(obj)
        error(obj.err_missing_symbol);        
    end
    
    % Query MTK for basis from symbolic representation.   
	[re, im] = mtk('generate_basis', ...
				   obj.Scenario.System.RefId, obj.SymbolCell);

 end
