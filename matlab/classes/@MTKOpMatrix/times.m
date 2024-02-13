function val = times(lhs, rhs)
% TIMES Element-wise multiplication .*
    [this, other, this_on_lhs] = mapThis(lhs, rhs);
         
    % Rescale?
    if (isnumeric(other) && numel(other) == 1)
        val = rescaleMatrix(this, other);
        return;
    end
    
    % Test if accelerated multiplication is possible    
    if isa(other, 'MTKPolynomial') || isa(other, 'MTKMonomial')
        this.checkSameScenario(other);
        assert(other.IsScalar, ...
            "Currently, operator matrices can only be multiplied by scalar objects");
        
        if (this_on_lhs)
            [res_id, res_dim, res_mono, res_herm] = ...
                mtk('multiply', this.Scenario.System.RefId, ...
                                this.Index, other.OperatorCell);
        else
            [res_id, res_dim, res_mono, res_herm] = ...
                mtk('multiply', this.Scenario.System.RefId, ...
                                other.OperatorCell, this.Index);
        end

        val = MTKOpMatrix(this.Scenario, res_id, res_dim, res_mono, res_herm);    
        
        % This might result in new symbols:
        this.Scenario.System.UpdateSymbolTable();
        
        return;
    end
       
    % Cast to mono/poly and try again ... probably won't work
    val = degradeAndCall(lhs, rhs, @times);
end