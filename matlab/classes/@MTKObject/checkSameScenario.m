function checkSameScenario(obj, other)
% CHECKSAMESCENARIO Raise an error if scenarios do not match.        

    assert(isa(other,'MTKObject'));
    if obj.Scenario ~= other.Scenario
        error(obj.err_mismatched_scenario);
    end
end
