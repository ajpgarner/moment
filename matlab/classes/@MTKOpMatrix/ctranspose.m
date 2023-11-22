function val = ctranspose(obj)
%CTRANSPOSE Get conjugate transpose of object.
    
    % Hermitian object does not change.
    if obj.IsHermitian
        val = obj;
    end
    
    % Degrade to mono/poly and call
    if obj.IsMonomial
        this_as_mono = MTKMonomial(obj);
        val = ctranspose(this_as_mono);
    else
        this_as_poly = MTKPolynomial(obj);
        val = ctranspose(this_as_poly);
    end
end

