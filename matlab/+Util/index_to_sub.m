function val = index_to_sub(sz, index)
%INDEX_TO_SUB Like ind2sub, but outputs array
    arguments
        sz(1,:) uint64 {mustBePositive}
        index (1,1) uint64 {mustBePositive}
    end
    
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

