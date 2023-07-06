function val = linfunc(obj, tensor)
% LINFUNC Element-wise sum of this object with supplied tensor.
%
% RETURN:
%   A scalar polynomial.
%
    arguments
        obj (:,:) MTKPolynomial
        tensor (:,:) double
    end
    
    if ~isequal(size(tensor), size(obj))
        error("Expected input tensor with dimension " + mat2str(size(obj)));
    end
    
    % Multiply coefficients by tensor
    product_polynomials = obj .* tensor;
    
    % No contraction if scalar
    if obj.IsScalar()
        val = product_polynomials;
        return;
    end
    
    % Contraction
    mask = ~cellfun(@isempty, product_polynomials.Constituents);
    non_zero = product_polynomials.Constituents(mask);
    if isempty(non_zero)
        val = MTKPolynomial.InitZero(obj.Scenario, [1 1]);
    else
        val = MTKPolynomial(obj.Scenario, vertcat(non_zero{:}));    
    end
end