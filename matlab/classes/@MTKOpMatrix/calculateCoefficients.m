function [re, im] = calculateCoefficients(obj)
    [re, im] = mtk('generate_basis', obj.Scenario.System.RefId, ...
                   obj.Index, 'monolith', 'sparse');               
end

