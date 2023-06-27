function val = ne(lhs, rhs)
% NE Compare LHS and RHS for value-wise inequality.
% Logical negation of eq(lhs, rhs)
%
% See also: MONOMIAL.EQ
%
    val = ~eq(lhs, rhs);
end