function [id, conjugated, re, im] = getDefaultSymbolInfo(obj)
    if obj.IsScalar()
        id = -1;
        conjugated = false;
        re = 0;
        im = 0;
    else
        dims = size(obj);
        id = int64(ones(dims)) * -1;
        conjugated = false(dims);               
        re = uint64(zeros(dims));
        im = uint64(zeros(dims));
    end            
end