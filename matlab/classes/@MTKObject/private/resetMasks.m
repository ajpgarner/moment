function resetMasks(obj)
    if obj.Scenario.HasMatrixSystem        
        obj.mask_re = false(1, obj.Scenario.System.RealVarCount);
        obj.mask_im = false(1, obj.Scenario.System.ImaginaryVarCount);
    else
        obj.mask_re = false(1,0);
        obj.mask_im = false(1,0);
    end
    
    obj.basis_elems_re = uint64.empty(1,0);
    obj.basis_elems_im = uint64.empty(1,0);    
    
    obj.has_cached_masks = false;
end