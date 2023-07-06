function obj = InitValue(setting, values)
    dimensions = size(values);
    obj = MTKPolynomial(setting, 'overwrite', dimensions);
    if obj.IsScalar
        assert(numel(values) == 1);
        obj.Constituents = MTKMonomial.InitValue(setting, values);
    else
        for idx = 1:numel(values)
            obj.Constituents{idx} = ...
                MTKMonomial.InitValue(setting, values(idx));
        end
    end
end