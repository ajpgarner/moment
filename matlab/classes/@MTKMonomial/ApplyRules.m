function val = ApplyRules(obj, rulebook )
% APPLYRULES Transform moments of matrix according to rulebook.
%
% Effectively applies rules to each constituent matrix in turn.
% 
    arguments
        obj (1,1) MTKMonomial
        rulebook (1,1) MomentRulebook
    end

    % Promote to polynomial, then apply
    obj_as_poly = MTKPolynomial(obj.Scenario, obj);
    val = obj_as_poly.ApplyRules(rulebook);
end 