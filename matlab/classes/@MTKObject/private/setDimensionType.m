function setDimensionType(obj)
%SETDIMENSIONTYPE Identify type of object from its dimensions.
    if numel(obj.dimensions) ~= 2
        obj.dimension_type = 4; % TENSOR
    elseif prod(obj.dimensions) == 1
        obj.dimension_type = 0; % SCALAR
    elseif obj.dimensions(1) == 1
        obj.dimension_type = 1; % ROW-VECTOR
    elseif obj.dimensions(2) == 1
        obj.dimension_type = 2; % COLUMN-VECTOR
    else
        obj.dimension_type = 3; % MATRIX
    end
end