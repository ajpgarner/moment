 function val = transpose(obj)
% CTRANSPOSE Transpose (without complex conjugation).

    % Transpose elements
    switch obj.DimensionType
        case 0 %SCALAR      
            val = obj;
            return
        case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
            trans_ops = obj.Operators.';
            new_coefs = obj.Coefficient.';
            hashes = obj.Hash.';
        otherwise
            permutation = [2, 1, 3:numel(obj.Dimensions)];
            trans_ops = permute(obj.Operators, permutation);
            new_coefs = permute(obj.Coefficient, permutation);
            hashes = permute(obj.Hash, permutation);                    
    end

    % Make new object
    val = MTKMonomial.InitForOverwrite(obj.Scenario,...
                                             size(trans_ops));
    val.Operators = trans_ops;
    val.Coefficient = new_coefs;
    val.Hash = hashes;
end