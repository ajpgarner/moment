 function val = length(obj)
% LENGTH Size of the longest dimension.
    if isempty(obj)
        val = 0;
    else
        val = max(obj.dimensions);
    end
end