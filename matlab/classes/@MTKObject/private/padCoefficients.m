 function padCoefficients(obj)
% PADCOEFFICIENTS Add zeros to end of coefficients.
    if ~obj.needs_padding
        return
    end

    assert(obj.has_cached_coefs);            
    switch(obj.dimension_type)
        case 0 % SCALAR
            padScalarCoefficients(obj)
        case 1 % ROW-VECTOR
            padVectorCoefficients(obj);
        case 2 % COL-VECTOR
            padVectorCoefficients(obj);
        otherwise % MATRIX or TENSOR
            padTensorCoefficients(obj);
    end

    % Flag padding as done
    obj.needs_padding = false;

    % Start listening again for further updates
    if ~isempty(obj.symbol_added_listener)
        obj.symbol_added_listener.Enabled = true;
    end
 end

%% Private functions
function padScalarCoefficients(obj)              
    % Pad real coefficients
    target_real = obj.Scenario.System.RealVarCount;
    delta_re = target_real - length(obj.real_coefs);
    if delta_re > 0
        obj.real_coefs = [obj.real_coefs; ...
                          complex(sparse(delta_re, 1))];
    elseif delta_re < 0
        error("Real co-efficients should not shrink!");
    end

    % Pad imaginary coefficients
    target_im = obj.Scenario.System.ImaginaryVarCount;
    delta_im = target_im - length(obj.im_coefs);
    if delta_im > 0
        obj.im_coefs = [obj.im_coefs; ...
                        complex(sparse(delta_im, 1))];
    elseif delta_im < 0
        error("Imaginary co-efficients should not shrink!");
    end
end

function padVectorCoefficients(obj)
    % Pad real coefficients
    target_real = obj.Scenario.System.RealVarCount;
    delta_re = target_real - size(obj.real_coefs, 1);
    if delta_re > 0
        obj.real_coefs = [obj.real_coefs; ...
                          complex(sparse(delta_re, numel(obj)))];
    elseif delta_re < 0
        error("Real co-efficients should not shrink!");
    end

    % Pad imaginary coefficients
    target_im = obj.Scenario.System.ImaginaryVarCount;
    delta_im = target_im - size(obj.im_coefs, 1);
    if delta_im > 0
        obj.im_coefs = [obj.im_coefs; ...
                        complex(sparse(delta_im, numel(obj)))];
    elseif delta_im < 0
        error("Imaginary co-efficients should not shrink!");
    end
end

function padTensorCoefficients(obj)            
    % Pad real coefficients
    target_real = obj.Scenario.System.RealVarCount;
    delta_re = target_real - size(obj.real_coefs{1}, 1);
    if delta_re > 0
        for idx=1:numels(obj.real_coefs)
            obj.real_coefs{idx} = [obj.real_coefs{idx}; ...
                          complex(sparse(delta_re, ...
                                         obj.dimensions(1)))];
        end
    elseif delta_re < 0
        error("Real co-efficients should not shrink!");
    end

    % Pad imaginary coefficients
    target_im = obj.Scenario.System.ImaginaryVarCount;
    delta_im = target_im - size(obj.im_coefs{1}, 1);
    if delta_im > 0                
        for idx=1:numels(obj.im_coefs)
            obj.im_coefs{idx} = [obj.im_coefs{idx}; ...
                            complex(delta_im, ...
                            sparse(obj.dimensions(1)))];
        end
    elseif delta_im < 0
        error("Imaginary co-efficients should not shrink!");
    end
end