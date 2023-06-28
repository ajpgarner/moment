function makeOperatorCell(obj)
%MAKEOPERATORCELL Create cell description of polynomial.
    if obj.IsScalar
        obj.operator_cell = cell(1, length(obj.Constituents));
        for idx = 1:length(obj.Constituents)
            obj.operator_cell{idx} = ...
                {obj.Constituents(idx).Operators, ...
                 obj.Constituents(idx).Coefficient};
        end
        obj.done_oc = true;
    else
        error("Operator cell not supported for non-scalar polynomials.");
    end            
end