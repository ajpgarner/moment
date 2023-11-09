function val = conj(obj)
% CONJ Conjugation (without transpose).
    [conj_ops, sign, hashes] = obj.Scenario.Conjugate(obj.Operators);    
    new_coefs = conj(obj.Coefficient) .* sign;

    val = MTKMonomial.InitForOverwrite(obj.Scenario, size(obj));
    val.Operators = conj_ops;
    val.Coefficient = new_coefs;
    val.Hash = hashes;
end