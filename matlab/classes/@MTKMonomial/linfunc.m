function val = linfunc(obj, tensor)
% LINFUNC Element-wise sum of this object with supplied tensor.
%
% RETURN:
%   A scalar polynomial.
%
    arguments
        obj (:,:) MTKMonomial
        tensor (:,:) double
    end
    
    if ~isequal(size(tensor), size(obj))
        error("Expected input tensor with dimension " + mat2str(size(obj)));
    end
    
    % Multiply coefficients by tensor, and remove elements close to zero
    prod_coef = obj.Coefficient .* tensor;
    [prod_coef , mask] = obj.Scenario.Prune(prod_coef);
    nnz_elems = nnz(~mask);
    prod_coef = prod_coef(~mask);
    nz_elems = find(~mask);
        
    % If all terms were zero, return null polynomial
    if nz_elems == 0
        val = MTKPolynomial.InitZero(obj.Scenario, [1 1]);
        return;
    end
   
    % Otherwise, construct appropriate non-zero polynomial
    if obj.IsScalar        
        assert(nnz_elems == 1);
        assert(nz_elems(1) == 1);
        components = MTKMonomial(obj.Scenario, obj.Operators, prod_coef);
    else
        if nnz_elems == 1
            components = MTKMonomial(obj.Scenario, ...
                                     obj.Operators{nz_elems(1)}, ...
                                     prod_coef);
        else
            components = MTKMonomial(obj.Scenario, 'overwrite', ...
                                     [nnz_elems, 1]);
            for idx=1:nnz_elems
                components.Operators{idx} = obj.Operators{nz_elems(idx)};
                components.Hash(idx) = obj.Hash(nz_elems(idx));
                components.Coefficient(idx) = prod_coef(idx);                
            end
        end            
    end    
    val = MTKPolynomial(obj.Scenario, components);    
end