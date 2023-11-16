function val = plus(lhs, rhs)
% PLUS Addition +

    % Special case, sometimes LocalizingMatrix + LocalizingMatrix = Matrix
    if isa(lhs,'MTKOpMatrix') && isa(rhs, 'MTKOpMatrix')
        
        % First, make polynomial localizing matrix if possible 
        val = combineToLM(lhs, rhs, false);
        if ~islogical(val)
            return;
        end
        
        % Otherwise invoke plus functionality       
        lhs.checkSameScenario(rhs);
        [index, dim, mono, herm] = ...
            mtk('plus', 'index', lhs.Scenario.System.RefId, ...
                lhs.Index, rhs.Index);
            
            
        val = MTKOpMatrix(lhs.Scenario, index, dim, mono, herm);
        return;
    end
    
    % Otherwise, downcast and call
    val = degradeAndCall(lhs, rhs, @plus);
    
end