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
            error("%d imaginary values expected, but %d were provided.", ...
                obj.Scenario.System.ImaginaryVarCount, im_vals);
        end
    else
        has_imaginary = false;
    end

    rec = obj.RealCoefficients;

    switch obj.dimension_type
        case 0 % SCALAR
            val = re_vals * rec;
        case 1 % ROW_VECTOR
            val = re_vals * rec;
        case 2 % COL_VECTOR
            val = transpose(re_vals * rec);
        otherwise
            error("Cannot yet apply to object of this type.");
    end

    if has_imaginary 
        imc = obj.ImaginaryCoefficients;
        switch obj.dimension_type
            case 0 % SCALAR
                val = val + (im_vals * imc);
            case 1 % ROW_VECTOR
                val = val + (im_vals * imc);
            case 2 % COL_VECTOR
                val = val + transpose(im_vals * imc);
            otherwise
                error("Cannot yet apply to object of this type.");
        end
    end            
end