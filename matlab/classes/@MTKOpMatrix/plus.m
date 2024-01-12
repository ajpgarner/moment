function val = plus(lhs, rhs)
% PLUS Addition +

    % Special case, sometimes LocalizingMatrix + LocalizingMatrix = Matrix
    if isa(lhs,'MTKOpMatrix') && isa(rhs, 'MTKOpMatrix')
        
        % First, attempt to make polynomial localizing matrix if possible 
        val = combineToLM(lhs, rhs, false);
        if ~islogical(val)
            return;
        end
        
        % Otherwise invoke matrix-matrix plus functionality       
        lhs.checkSameScenario(rhs);
        [index, dim, mono, herm] = ...
            mtk('plus', 'index', lhs.Scenario.System.RefId, ...
                lhs.Index, rhs.Index);
            
            
        val = MTKOpMatrix(lhs.Scenario, index, dim, mono, herm);
        return;
    end
    
    % Determine which of LHS is an MTKOpMatrix 
    if isa(lhs, 'MTKOpMatrix')
        this = lhs;
        other = rhs;
    else 
        this = rhs;
        other = lhs;
    end
    
    % If other is numeric, promote to MTKMonomial
    if isnumeric(other)
        other = MTKMonomial.InitValues(this.Setting, other);
    end
    
    % Check for compatibility
    assert(isa(other, 'MTKObject'), ...
           "Cannot add object of type %s to MTKOpMatrix", class(other));
    
    % Get cell representation of other object
    if this.Scenario.PermitsSymbolAliases && other.FoundAllSymbols
        other_cell = other.SymbolCell;
    elseif this.Scenario.DefinesOperators
        other_cell = other.OperatorCell;
    else
        error("Cannot add object which neither has symbols or operators");
    end
    
    % Pass to MTK to do matrix-scalar or matrix-array addition:
    [id, dim, is_mono, is_herm] = ...
       mtk('plus', ref_id, this.Index, other_cell); 
    
    val = MTKOpMatrix(this.Scenario, id, dim, is_mono, is_herm);
end