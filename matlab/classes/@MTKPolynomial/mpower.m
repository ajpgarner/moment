function val = mpower(lhs,rhs)
% MPOWER Matrix power

    % FIXME
    assert(lhs.IsScalar);
	
    if nargin < 2 || ~isnumeric(rhs) ...
		|| rhs <= 0 || rhs ~= floor(rhs)
        error("Invalid exponent.");
    end

    val = lhs;
    for i=1:rhs-1
        val = times(val, lhs);
    end
end
