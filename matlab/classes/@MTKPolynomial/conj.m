function val = conj(obj)
% CONJ Complex conjugation (without transpose).
%
% See also: POLYNOMIAL.TRANPOSE, POLYNOMIAL.CTRANPOSE

    if obj.IsScalar
        val = MTKPolynomial(obj.Scenario, ...
                                  conj(obj.Constituents));
    else
        new_constituents = cellfun(@(x) conj(x), ...
                                   obj.Constituents, ...
                                   'UniformOutput', false);
        val = MTKPolynomial(obj.Scenario, new_constituents);
    end
 end
