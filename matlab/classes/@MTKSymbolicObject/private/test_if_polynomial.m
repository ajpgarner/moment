function is_polynomial = test_if_polynomial(obj)
    is_polynomial = cellfun(@(x) numel(x) > 1, obj.SymbolCell);
    is_polynomial = any(is_polynomial(:));
end