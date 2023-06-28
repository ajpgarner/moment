 function val = uminus(obj)
% UMINUS Unary minus.
% Creates new polynomial, with all coefficients negated.
%
    if obj.IsScalar
        val = MTKPolynomial(obj.Scenario, -obj.Constituents);
    else
        val = MTKPolynomial(obj.Scenario, ...
                 cellfun(@(x) unminus(x), obj.Constituents,...
                         'UniformOutput', false));
    end            
end