function result = is_scalar_zero(value, tolerance)
%IS_SCALAR_ZERO True if value is numeric, scalar, and close to zero.
    if nargin < 2
        tolerance = 2 * eps(1.0);
    end
    
    if ~isnumeric(value) || ~isscalar(value)
        result = false;
        return;
    end
    
    result = abs(value) < tolerance;    
end

