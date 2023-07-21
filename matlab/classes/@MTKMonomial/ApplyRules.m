function val = ApplyRules(obj, rulebook )
% APPLYRULES Transform moments of matrix according to rulebook.
%
% Effectively applies rules to each constituent object in turn.

    % Promote to polynomial, then apply
    obj_as_poly = MTKPolynomial(obj);
    val = obj_as_poly.ApplyRules(rulebook);
        
end 