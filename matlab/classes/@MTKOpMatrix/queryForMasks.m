function [mask_re, mask_im, elems_re, elems_im] = queryForMasks(obj)
    [mask_re, mask_im, elems_re, elems_im] = ...
        mtk('operator_matrix', 'masks', ...
            obj.Scenario.System.RefId, obj.Index);
    
end

