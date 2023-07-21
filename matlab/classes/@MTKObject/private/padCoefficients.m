 function padCoefficients(obj)
% PADCOEFFICIENTS Add zeros to end of coefficients.
    if ~obj.needs_padding
        return
    end

    % Pad coefficients
    if obj.has_cached_coefs    
        doPadCoefficients(obj);
    end
    
    % Pad masks
    if obj.has_cached_masks
        padMasks(obj);
    end

    % Flag padding as done
    obj.needs_padding = false;

    % Start listening again for further updates
    if ~isempty(obj.symbol_added_listener)
        obj.symbol_added_listener.Enabled = true;
    end
 end

%% Private functions
function doPadCoefficients(obj)              
    % Pad real coefficients
    target_real = obj.Scenario.System.RealVarCount;
    delta_re = target_real - size(obj.real_coefs, 1);
    if delta_re > 0
        obj.real_coefs = [obj.real_coefs; ...
                          sparse(delta_re, numel(obj))];
    elseif delta_re < 0
        error("Real co-efficients should not shrink!");
    end

    % Pad imaginary coefficients
    target_im = obj.Scenario.System.ImaginaryVarCount;
    delta_im = target_im - size(obj.im_coefs, 1);
    if delta_im > 0
        obj.im_coefs = [obj.im_coefs; ...
                        sparse(delta_im, numel(obj))];
    elseif delta_im < 0
        error("Imaginary co-efficients should not shrink!");
    end
end

function padMasks(obj)
    % Pad real mask
    target_real = obj.Scenario.System.RealVarCount;
    delta_re = target_real - numel(obj.mask_re);
    if delta_re > 0
        if issparse(obj.mask_re)
            obj.mask_re = [obj.mask_re, logical(sparse(1, delta_re))];
        else
            obj.mask_re = [obj.mask_re, false(1, delta_re)];
        end
    elseif delta_re < 0
        error("Real co-efficients should not shrink!");
    end

    % Pad imaginary mask
    target_im = obj.Scenario.System.ImaginaryVarCount;
    delta_im = target_im - numel(obj.mask_im);
    if delta_im > 0
        if issparse(obj.mask_im)
            obj.mask_im = [obj.mask_im, logical(sparse(1, delta_im))];
        else
            obj.mask_im = [obj.mask_im, false(1, delta_im)];
        end
    elseif delta_im < 0
        error("Imaginary co-efficients should not shrink!");
    end
end