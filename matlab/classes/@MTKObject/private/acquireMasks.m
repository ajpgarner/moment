function acquireMasks(obj)
    if ~obj.Scenario.HasMatrixSystem
        error("Masks cannot be calculated before matrix system is initialized.");
    end
    
     [obj.mask_re, obj.mask_im, obj.basis_elems_re, obj.basis_elems_im] ...
         = queryForMasks(obj);
     
     obj.has_cached_masks = true;
end