 function val = Apply(obj, re_vals, im_vals)
% APPLY Combine vector inputs with object coefficients.
% Calculates: re_vals * re_coefs  + im_vals * im_coefs
% 
% PARAMS
%   re_vals - (Row) vector of values to combine with real coefficients.
%   im_vals - (Row) vector of values to combine with imaginary coefficients.
%
% RETURNS
%  Object of the same type as re_vals; e.g. numeric or sdpvar.
%  Shape of output matches size(obj).
%

    % Check and sanitize real inputs
    re_vals = reshape(re_vals, 1, []);
    if numel(re_vals) ~= obj.Scenario.System.RealVarCount
        error("%d real values expected, but %d were provided.", ...
            obj.Scenario.System.RealVarCount, re_vals);
    end

    % Check and sanitize imaginary inputs if any
    if nargin >= 3
        has_imaginary = true;
        im_vals = reshape(im_vals, 1, []);
        if numel(im_vals) ~= obj.Scenario.System.ImaginaryVarCount
            error("%d imaginary values expected, but %d were provided.",...
                obj.Scenario.System.ImaginaryVarCount, im_vals);
        end
    else
        has_imaginary = false;
    end

    % Query for co-efficients.
    re_coefs = obj.RealCoefficients;

    switch obj.dimension_type
        case 0 % SCALAR
            val = re_vals * re_coefs;
        case 1 % ROW_VECTOR
            val = re_vals * re_coefs;
        case 2 % COL_VECTOR
            val = transpose(re_vals * re_coefs);
        otherwise
            cell_dims = num2cell(size(obj));
            val = reshape(re_vals * re_coefs, cell_dims{:});
    end

    if has_imaginary 
        % Query for co-efficients
        im_coefs = obj.ImaginaryCoefficients;
        switch obj.dimension_type
            case 0 % SCALAR
                val = val + (im_vals * im_coefs);
            case 1 % ROW_VECTOR
                val = val + (im_vals * im_coefs);
            case 2 % COL_VECTOR
                val = val + transpose(im_vals * im_coefs);
            otherwise
                val = val + reshape(im_vals * im_coefs, cell_dims{:});
        end
    end            
end