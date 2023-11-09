function val = ctranspose(obj)
% CTRANSPOSE Complex conjugation / transpose.

    % Do conjugation of operators and coefficients
    [conj_ops, sign, hashes] = obj.Scenario.Conjugate(obj.Operators);    
    new_coefs = conj(obj.Coefficient) .* sign;

    % Transpose elements
    switch obj.DimensionType
        case 0 %SCALAR
        case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
            conj_ops = conj_ops.';                   
            new_coefs = new_coefs.';
            hashes= hashes.';                    
        otherwise
            permutation = [2, 1, 3:numel(obj.Dimensions)];
            conj_ops = permute(conj_ops, permutation);
            new_coefs = permute(new_coefs, permutation);
            hashes = permute(hashes, permutation);                    
    end

    % Make new object
    if obj.IsScalar
        new_size = [1, 1];
    else
        new_size = size(conj_ops);
    end
    
    val = MTKMonomial.InitForOverwrite(obj.Scenario, new_size);
    val.Operators = conj_ops;
    val.Coefficient = new_coefs;
    val.Hash = hashes;
end
