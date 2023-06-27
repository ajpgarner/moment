function val = conj(obj)
% CONJ Conjugation (without transpose).
    [conj_ops, negated, hashes] = obj.Scenario.Conjugate(obj.Operators);
    neg_mask = ones(size(negated));
    neg_mask(negated) = -1;
    new_coefs = conj(obj.Coefficient) .* neg_mask;

    val = MTKMonomial.InitForOverwrite(obj.Scenario, size(obj));
    val.Operators = conj_ops;
    val.Coefficient = new_coefs;
    val.Hash = hashes;
end