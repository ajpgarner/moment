function val = numel(obj)
% NUMEL Number of elements.
    if isempty(obj)
        val = 0;
        return;
    end
    val = prod(obj.dimensions);
end