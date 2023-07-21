function val = mpower(lhs,rhs)
    
    % FIXME
    assert(lhs.IsScalar);
	
    if nargin < 2 || rhs <= 0 || rhs ~= floor(rhs)
        error("Invalid exponent.");
    end

    val = lhs;
    for i=1:rhs-1
        val = mtimes(val, lhs);
    end
end