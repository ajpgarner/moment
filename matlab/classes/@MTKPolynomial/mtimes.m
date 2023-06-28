 function val = mtimes(lhs, rhs)
% MTIMES Multiplication.
%
% SYNTAX
%   1. v = polynomial * double
%   2. v = double * polynomial 
%   3. v = polynomial * monomial
%   4. v = monomial * polynomial 
%   5. v = polynomial_A * polynomial_B
%
% RETURNS
%   A new MTKPolynomial with appropriate coefficients and
%   operators.
%
% See also: POLYNOMIAL.TIMES, MONOMIAL.MTIMES
%

    % Alias for .* if either side is a scalar.
    if numel(lhs)==1 || numel(rhs)==1
        val = times(lhs, rhs);
        return;
    end

    % TODO: Matrix multiplication.            
    error(['Matrix multiplication not yet defined for polynomials. ',...
           'For element-wise multiplication, use _.*_ instead.']);
end
