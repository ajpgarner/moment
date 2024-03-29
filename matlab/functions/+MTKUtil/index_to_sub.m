function val = index_to_sub(sz, index)
%INDEX_TO_SUB Like ind2sub, but outputs array
    
    assert(nargin == 2);
    sz = reshape(uint64(sz), 1, []);
    index = uint64(index);
    dimension = length(sz);
    if index > prod(sz)
        error("Index out of bounds");
    end
    
    val = zeros(1, dimension);
    r = index - 1;
    
    for d = 1:dimension
        val(d) = mod(r, sz(d)) + 1;
        r = idivide(r, sz(d));
    end
end

