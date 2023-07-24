function [re, im] = calculateCoefficients(obj)
    % Query MTK for basis from symbolic representation.
    [re, im] = mtk('generate_basis', obj.Scenario.System.RefId, ...
                   obj.SymbolCell);

end