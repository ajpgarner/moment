function mode = spliceIn(obj, indices, value)
    
    % Promote RHS to Polynomial if Monomial
    if isa(value, 'MTKMonomial')
        value = MTKPolynomial(value);
    elseif isnumeric(value) % Promote RHS to Polynomial if numeric
        value = MTKPolynomial.InitValue(obj.Scenario, value);
    end
        
    mode = spliceIn@MTKObject(obj, indices, value);
  
    switch mode
        case 0
            poly_splice_scalar(obj, indices, value);
        case 1
            poly_splice_array(obj, indices, value);
        case 2
            poly_splice_array(obj, indices, value);
        case 3
            poly_splice_broadcast(obj, indices, value);
    end
    
    % FIXME: Symbol cell / operator cell splicing
    obj.done_sc = false;
    obj.done_oc = false;
    
    
end

%% Private functions
function obj = poly_splice_scalar(obj, indices, value)
    if obj.IsScalar
        obj.Constituents = value.Constituents;
    else
        obj.Constituents{indices{:}} = value.Constituents;
    end
end

function obj = poly_splice_array(obj, indices, value)
    [obj.Constituents{indices{:}}] = deal(value.Constituents{:});
end

function obj = poly_splice_broadcast(obj, indices, value)
    [obj.Constituents{indices{:}}] = deal(value.Constituents);
end