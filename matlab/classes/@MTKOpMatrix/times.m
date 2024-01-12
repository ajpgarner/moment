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
        this.checkSameScenario(other.Scenario);
        assert(other.IsScalar, ...
            "Currently, operator matrices can only be multiplied by scalar objects");
        
        if (this_on_lhs)
            [res_id, res_dim, res_mono, res_herm] = ...
                mtk('multiply', obj.Scenario.System.RefId, ...
                                obj.Index, other.OperatorCell);
        else
            [res_id, res_dim, res_mono, res_herm] = ...
                mtk('multiply', obj.Scenario.System.RefId, ...
                                other.OperatorCell, obj.Index);
        end

        val = MTKOpMatrix(obj.Scenario, res_id, res_dim, res_mono, res_herm);    
        return;
    end
       
    % Cast to mono/poly and try again ... probably won't work
    val = degradeAndCall(lhs, rhs, @times);
end