function val = ApplyRules(obj, rulebook)
% APPLYRULES Transform moments of matrix according to rulebook.
%
% Effectively applies rules to each constituent matrix in turn.
% 
    arguments
        obj (1,1) MTKPolynomial
        rulebook (1,1) MomentRulebook
    end

    % Scenarios must match
    if (rulebook.Scenario ~= obj.Scenario)
        error(obj.err_mismatched_scenario);
    end

    % Get transformed version of polynomial
    as_symbol_cell = obj.SymbolCell();
    output_sequences = mtk('apply_moment_rules', ...
        obj.Scenario.System.RefId, rulebook.RulebookId, ...
        'output', 'sequences', as_symbol_cell);

    % Construct new, transformed polynomial
    val = MTKPolynomial(obj.Scenario, output_sequences);

    % Degrade to monomial if only single element.
    if length(val.Constituents) == 1
        val = val.Constituents(1);
    end
end