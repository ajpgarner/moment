function val = transpose(obj)
% CTRANSPOSE Transpose elements (without complex conjugation).
%
% For tensors larger than matrices, swaps indices 1 and 2.
%
% See also: POLYNOMIAL.CONJ, POLYNOMIAL.CTRANPOSE

    switch obj.DimensionType
        case 0 %SCALAR    
            val = obj;
            return;
        case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
            trans_const = obj.Constituents.';
        otherwise
            permutation = [2, 1, 3:numel(obj.Dimensions)];
            trans_const = permute(obj.Constituents, permutation);
    end

    % Make new object
    val = MTKPolynomial(obj.Scenario, trans_const);            
end
