function resetCoefficients(obj)
% RESETCOEFFICIENTS Forget calculated co-efficients.
    switch obj.dimension_type
        case 0 % SCALAR
            obj.real_coefs = complex(sparse(1,0));
            obj.im_coefs = complex(sparse(1,0));
        case 1 % ROW-VECTOR
            obj.real_coefs = complex(sparse(0,0));
            obj.im_coefs = complex(sparse(0,0));
        case 2 % COLUMN-VECTOR
            obj.real_coefs = complex(sparse(0,0));
            obj.im_coefs = complex(sparse(0,0));
        case 3 % MATRIX
            obj.real_coefs = cell(obj.dimensions(2), 1);
            obj.im_coefs = cell(obj.dimensions(2), 1);
        case 4 % TENSOR
            obj.real_coefs = cell(obj.dimensions(2:end));
            obj.im_coefs = cell(obj.dimensions(2:end));
    end

    obj.has_cached_coefs = false;
    obj.needs_padding = false;

    if ~isempty(obj.symbol_added_listener)
        obj.symbol_added_listener.Enabled = false;
    end           
end