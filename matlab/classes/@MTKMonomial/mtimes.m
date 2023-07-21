function val = mtimes(lhs, rhs)
% MTIMES Matrix multiplication.
%
% SYNTAX
%   1. v = monomial * double
%   2. v = double * monomial
%   3. v = monomial * monomial
%
% RETURNS
%   A new MTKMonomial with appropriate coefficients and
%   operators; except when double = 0, then 0.
%
% Due to class precedence, poly * mono and mono * poly cases are 
% handled by MTKPolynomial.mtimes.
%
% See also: ALGEBRAIC.POLYNOMIAL.MTIMES.
%

    % Alias for .* if either side is a scalar.
    if numel(lhs)==1 || numel(rhs)==1
        val = times(lhs, rhs);
        return;
    end

    % TODO: Matrix multiplication.            
    error(['Matrix multiplication not yet defined for monomials. ',...
           'For element-wise multiplication, use _.*_ instead.']);
end