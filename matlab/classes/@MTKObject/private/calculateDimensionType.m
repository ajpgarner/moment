function val = calculateDimensionType(obj)
%SETDIMENSIONTYPE Identify type of object from its dimensions.
    dimensions = size(obj);
    if numel(dimensions) ~= 2
        val = 4; % TENSOR
        return;
    elseif prod(dimensions) == 1
        val = 0; % SCALAR
    elseif dimensions(1) == 1
        val = 1; % ROW-VECTOR
    elseif dimensions(2) == 1
        val = 2; % COLUMN-VECTOR
    else
        val = 3; % MATRIX
    end
end