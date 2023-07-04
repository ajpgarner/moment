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
    
    % TODO: Contraction
    
    error("TODO");
    
    val = MTKPolynomial(obj.Scenario, components);    
end