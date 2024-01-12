function val = rescaleMatrix(obj, factor)
%RESCALEMATRIX Multiply this matrix by a scalar.
% This function should be overloaded for specific rescale behaviour -
% default behaviour will be to cast factor to monomial and call `multiply`.
    
    assert(isnumeric(factor) && numel(factor)==1, ...
           "Factor must be numeric scalar.");
    
    other_as_mono = MTKMonomial.InitValue(obj.Scenario, factor);
    [res_id, res_dim, res_mono, res_herm] = ...
                mtk('multiply', obj.Scenario.System.RefId, ...
                    obj.Index, other_as_mono.OperatorCell);

    val = MTKOpMatrix(obj.Scenario, res_id, res_dim, res_mono, res_herm);    
end

