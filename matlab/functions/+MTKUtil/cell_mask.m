function output = cell_mask(cell_array, mask, assign_value)
% CELL_MASK Apply value across cell array at logical indices
    assert(isequal(size(cell_array), size(mask)));
    output = cell_array;
    if any(mask(:))
        output(mask) = deal(repelem({assign_value}, sum(mask(:))));
    end
end

