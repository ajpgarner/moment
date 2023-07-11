function val = ApplyRules(obj, rulebook)
%APPLYRULES Apply rules to this MTKObject.
% Should be overloaded by child classes.

    % Validate input
    assert(nargin == 2 && isa(rulebook, 'MTKMomentRulebook'));

    % Scenarios must match
    if (rulebook.Scenario ~= obj.Scenario)
        error(obj.err_mismatched_scenario);
    end
    
    % Construct substituted matrix
    [index, dim, mono, herm] = ...
        mtk('substituted_matrix', obj.Scenario.System.RefId, ...
            obj.Index, rulebook.Id);        
    val = MTKOpMatrix(obj.Scenario, index, dim, mono, herm);
end

