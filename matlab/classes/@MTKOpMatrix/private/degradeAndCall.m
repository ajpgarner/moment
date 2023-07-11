function val = degradeAndCall(lhs, rhs, functor)
%DEGRADEANDCALL Cast object to monomial/polynomial and call function.

    [this, other, this_on_lhs] = mapThis(lhs, rhs);

    if this.IsMonomial
        this_as_mono = MTKMonomial(this);
        if this_on_lhs
            val = functor(this_as_mono, other);
        else
            val = functor(other, this_as_mono);
        end
    else
        this_as_poly = MTKPolynomial(this);
        if this_on_lhs
            val = functor(this_as_poly, other);
        else
            val = functor(other, this_as_poly);
        end
    end
end

