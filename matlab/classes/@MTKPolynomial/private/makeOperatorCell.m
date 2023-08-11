function makeOperatorCell(obj)
%MAKEOPERATORCELL Create cell description of polynomial.
    if obj.IsScalar
        obj.operator_cell = {makeOneOperatorCell(obj.Constituents)};
    else
        obj.operator_cell = ...
            cellfun(@makeOneOperatorCell, obj.Constituents, ...
                    'UniformOutput', false);
    end 
    obj.done_oc = true;
end

%% Private functions
function val = makeOneOperatorCell(constituents)
    assert(isa(constituents, 'MTKMonomial'));
    val = cell(1, numel(constituents));
    
    if constituents.IsScalar 
        val{1} = {constituents.Operators, constituents.Coefficient};
    else
        for pIdx = 1:numel(constituents)
            val{pIdx} = {constituents.Operators{pIdx}, ...
                         constituents.Coefficient(pIdx)};
        end
    end    
end
