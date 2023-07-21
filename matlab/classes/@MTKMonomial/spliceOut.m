function spliceOut(output, source, indices)
    spliceOut@MTKObject(output, source, indices);

    if source.IsScalar
        assert(output.IsScalar);
        output.Operators = source.Operators;
    else
        output.Operators = source.Operators(indices{:});
    end
    output.Coefficient = source.Coefficient(indices{:});
    output.Hash = source.Hash(indices{:});

    output.symbol_id = source.symbol_id(indices{:});
    output.symbol_conjugated = source.symbol_conjugated(indices{:});
    output.re_basis_index = source.re_basis_index(indices{:});
    output.im_basis_index = source.im_basis_index(indices{:});

end
