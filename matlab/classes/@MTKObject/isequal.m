function result = isequal(lhs, rhs)
% ISEQUAL Test if two objects are equal.
%
% Unlike EQ, this function is single valued even when acting on arrays,
% and returns true if and onyly if the sizes of LHS and RHS are the same, 
% and every constituent element is the same.
%
% SEE ALSO: MONOMIAL.EQ, POLYNOMIAL.EQ
%
    if ~builtin('isequal', size(lhs), size(rhs))
        result = false;
        return
    end
    
    result = all(eq(lhs, rhs), 'all');
end

