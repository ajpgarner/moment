function val = makeOperatorCell(obj)
%MAKEOPERATORCELL Create operator cell description of monomial(s)
    val = cell(1, numel(obj));
    
    if obj.IsScalar 
        val{1} = {obj.Operators, obj.Coefficient};
    else
        for pIdx = 1:numel(obj)
            val{pIdx} = {obj.Operators{pIdx}, ...
                         obj.Coefficient(pIdx)};
        end
    end
end
