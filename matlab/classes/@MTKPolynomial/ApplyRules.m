function val = ApplyRules(obj, rulebook)
% APPLYRULES Transform moments of matrix according to rulebook.
%
% Effectively applies rules to each constituent matrix in turn.
% 

    % Scenarios must match
    if (rulebook.Scenario ~= obj.Scenario)
        error(obj.err_mismatched_scenario);
    end

    % Get transformed version of polynomial
    as_symbol_cell = obj.SymbolCell();
    output_op_cell = mtk('apply_moment_rules', ...
        obj.Scenario.System.RefId, rulebook.Id, ...
        'output', 'polynomials', as_symbol_cell);
    
    % Construct new, transformed polynomial
    val = MTKPolynomial(obj.Scenario, output_op_cell);

end