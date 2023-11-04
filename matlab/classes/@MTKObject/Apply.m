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
    if value(re_vals) == 0
        re_coefs = 0;
    elseif numel(re_vals) ~= obj.Scenario.System.RealVarCount
        error("%d real values expected, but %d were provided.", ...
            obj.Scenario.System.RealVarCount, numel(re_vals));
    else
        re_vals = reshape(re_vals, 1, []);
        re_coefs = obj.RealCoefficients;
    end

    % Check and sanitize imaginary inputs if any

    if nargin == 3
        if numel(im_vals) ~= obj.Scenario.System.ImaginaryVarCount
            error("%d imaginary values expected, but %d were provided.",...
                obj.Scenario.System.ImaginaryVarCount, numel(im_vals));
        else
            im_vals = reshape(im_vals, 1, []);
            im_coefs = obj.ImaginaryCoefficients;
        end
    else
        im_vals = 0;
        im_coefs = 0;
    end

    % Query for co-efficients.

    switch obj.dimension_type
        case 0 % SCALAR
            val = (re_vals * re_coefs) + (im_vals * im_coefs);
        case 1 % ROW_VECTOR
            val = (re_vals * re_coefs) + (im_vals * im_coefs);
        case 2 % COL_VECTOR
            val = transpose(re_vals * re_coefs + im_vals * im_coefs);
        otherwise
            cell_dims = num2cell(size(obj));
            val = reshape(re_vals * re_coefs + im_vals * im_coefs, cell_dims{:});
    end

end
