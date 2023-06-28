  function val = minus(lhs, rhs)
% MINUS Subtraction. Equivalent to addition of lhs with -rhs.
%
% See also: ALGEBRAIC.POLYNOMIAL.PLUS, ALGEBRAIC.POLYNOMIAL.UMINUS.
%
    val = lhs + (-rhs);
end