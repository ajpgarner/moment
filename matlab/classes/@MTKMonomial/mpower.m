function val = mpower(lhs,rhs)
    arguments
        lhs (1,1) MTKMonomial
        rhs (1,1) double
    end

    % FIXME
    assert(lhs.IsScalar);

    if rhs <= 0 || rhs ~= floor(rhs)
        error("Invalid exponent.");
    end

    val = lhs;
    for i=1:rhs-1
        val = mtimes(val, lhs);
    end
end