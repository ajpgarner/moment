function val = ne(lhs, rhs)
% NE Compare LHS and RHS for value-wise inequality.
% Logical negation of eq(lhs, rhs)
%
% See also: POLYNOMIAL.EQ
%
    val = ~eq(lhs, rhs);
end