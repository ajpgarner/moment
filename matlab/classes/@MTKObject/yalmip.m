function ym_expr = yalmip(obj, real_basis, im_basis)
% YALMIP Convert object to a (complex) YALMIP expression.
%
% Wraps Apply with checks for yalmip sdpvar object inputs.
%
% See also: SYMBOLIC.COMPLEXOBJECT.APPLY
%

    if nargin >= 2
        if ~isa(real_basis, 'sdpvar')
            if (real_basis ~= 0)
                error("Expected YALMIP sdpvar for real basis input.");
            end
        end
    else
        error("Expected at least one YALMIP sdpvar basis input.");
    end

    if nargin >=3 && ~isempty(im_basis)
        if ~isa(im_basis, 'sdpvar')
            error("Expected YALMIP sdpvar for imaginary basis input.");
        end
        ym_expr = obj.Apply(real_basis, im_basis);
    else
        ym_expr = obj.Apply(real_basis);
    end
end
