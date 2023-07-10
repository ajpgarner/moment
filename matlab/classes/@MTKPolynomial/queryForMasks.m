function [mask_re, mask_im, elems_re, elems_im] = queryForMasks(obj)
%QUERYFORMASKS Get object real and imaginary masks.

    if obj.IsScalar
        mask_re = obj.Constituents.RealMask;
        mask_im = obj.Constituents.ImaginaryMask;
        elems_re = obj.Constituents.RealBasisElements;
        elems_im = obj.Constituents.ImaginaryBasisElements;
    else
        mask_re = logical(sparse(1, obj.Scenario.System.RealVarCount));
        mask_im = logical(sparse(1, obj.Scenario.System.ImaginaryVarCount));
        
        for idx = 1:numel(obj)
            c_mask_re = obj.Constituents{idx}.RealMask;
            c_mask_im = obj.Constituents{idx}.ImaginaryMask;            
            
            mask_re = mask_re | c_mask_re;
            mask_im = mask_im | c_mask_im;
        end
        
        elems_re = uint64(find(mask_re));
        if isempty(elems_re)
            elems_re = uint64.empty(1,0);
        else
            elems_re = reshape(elems_re, 1, []);
        end
        
        elems_im = uint64(find(mask_im));
        if isempty(elems_im)
            elems_im = uint64.empty(1,0);
        else
            elems_im = reshape(elems_im, 1, []);
        end
        
    end
end

