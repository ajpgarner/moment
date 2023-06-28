function val = ctranspose(obj)
% CTRANSPOSE Complex conjugation / transpose.
%
% For tensors larger than matrices, indices 1 and 2 are exchanged.
%
% See also: POLYNOMIAL.CONJ, POLYNOMIAL.TRANPOSE
%

        % Conjugate each polynomial's constituents
        if obj.IsScalar
            conj_const = conj(obj.Constituents);
        else
            conj_const = cellfun(@(x) conj(x), obj.Constituents, ...
                                 'UniformOutput', false);
        end

        % Transpose elements
        switch obj.DimensionType
            case 0 %SCALAR                                
            case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
                conj_const = conj_const.';
            otherwise
                permutation = [2, 1, 3:numel(obj.Dimensions)];
                conj_const = permute(conj_const, permutation);
        end

        % Make new object
        val = MTKPolynomial(obj.Scenario, conj_const);            
    end