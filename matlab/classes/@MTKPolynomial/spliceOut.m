 function spliceOut(output, source, indices)
 
    spliceOut@MTKObject(output, source, indices);

    if source.IsScalar
        assert(output.IsScalar);
        output.Constituents = source.Constituents;
    else
        output.Constituents = source.Constituents(indices{:});
    end
end