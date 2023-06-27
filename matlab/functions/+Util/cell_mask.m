function output = cell_mask(cell_array, mask, assign_value)
% CELL_MASK Apply value across cell array at logical indices
arguments
    cell_array cell
    mask logical
    assign_value = []
end
    assert(isequal(size(cell_array), size(mask)));
    output = cell_array;
    if any(mask, 'all')
        output(mask) = deal(repelem({assign_value}, sum(mask, 'all')));    
    end
end

