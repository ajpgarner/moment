function val = times(lhs, rhs)
% TIMES Element-wise multiplication .*
    [this, other, this_on_lhs] = mapThis(lhs, rhs);
    
    % Test if accelerated multiplication is possible    
    if isa(other, 'MTKPolynomial') || isa(other, 'MTKMonomial')
        assert(other.Scenario == this.Scenario, ...
               "Settings must match to multiply objects.");

        if this.IsMonomial && other.FoundAllSymbols
           if this_on_lhs
               [res_id, res_dim, res_mono, res_herm] = ...
                   mtk('multiply', this.Scenario.System.RefId, ...
                       this.Index, other.SymbolCell);
           else
               [res_id, res_dim, res_mono, res_herm] = ...
                   mtk('multiply', this.Scenario.System.RefId, ...
                       other.SymbolCell, this.Index);
           end
           val = MTKOpMatrix(this.Scenario, res_id, ...
                             res_dim, res_mono, res_herm);
           return
        end    
    end
    
    % Rescale?
    if (isnumeric(lhs) && numel(lhs) == 1)
        val = rescaleMatrix(rhs, lhs);
        return;
    elseif (isnumeric(rhs) && numel(rhs) == 1)
        val = rescaleMatrix(lhs, rhs);
        return;
    end
    
    % Cast to mono/poly
    val = degradeAndCall(lhs, rhs, @times);
end