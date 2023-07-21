function output = cell_mask(cell_array, mask, assign_value)
% CELL_MASK Apply value across cell array at logical indices

	assert(nargin>=2);
	if (nargin < 3)
		assign_value = [];
	end

    assert(isequal(size(cell_array), size(mask)));
    output = cell_array;
    if any(mask, 'all')
        output(mask) = deal(repelem({assign_value}, sum(mask, 'all')));    
    end
end

