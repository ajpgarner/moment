function val = ctranspose(obj)
%CTRANSPOSE Get conjugate transpose of object.
    
    % Hermitian object does not change.
    if obj.Hermitian
        val = obj;
    end
    
    % Degrade to mono/poly and call
    if this.IsMonomial
        this_as_mono = MTKMonomial(this);
        val = ctranpose(this_as_mono);
    else
        this_as_poly = MTKPolynomial(this);
        val = ctranpsoe(this_as_poly);
    end
end

