 function val = minus(lhs, rhs)
% MINUS Subtraction, defined as addition of lhs with -rhs.
%
% See also: ALGEBRAIC.MONOMIAL.PLUS, ALGEBRAIC.MONOMIAL.UMINUS
%
    val = lhs + -rhs;
end