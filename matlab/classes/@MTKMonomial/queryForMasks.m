function [mask_re, mask_im, elems_re, elems_im] = queryForMasks(obj)
%QUERYFORMASKS Get object real and imaginary masks.

    elems_re = unique(reshape(obj.RealBasisIndex, 1, []));
    elems_re = elems_re(elems_re > 0);
    if isempty(elems_re)
        elems_re = uint64.empty(1,0);
    end
    
    elems_im = unique(reshape(obj.ImaginaryBasisIndex, 1, []));
    elems_im = elems_im(elems_im > 0);
    if isempty(elems_im)
        elems_im = uint64.empty(1,0);
    end
    
    mask_re = logical(sparse(1, obj.Scenario.System.RealVarCount));
    mask_re(elems_re) = true;
    
    mask_im = logical(sparse(1, obj.Scenario.System.ImaginaryVarCount));
    mask_im(elems_im) = true;   
end

