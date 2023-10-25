 function cvx_expr = cvx(obj, real_basis, im_basis)
% CVX Convert object to a (complex) CVX expression.
%
% Wraps Apply with checks for CVX expression inputs.
%
% See also: MTKOBJECT.APPLY
%            
    if nargin >= 2
        if ~isa(real_basis, 'cvx')
            error("Expected CVX vector for real basis input.");
        end
    else
        error("Expected at least one CVX basis vector input.");
    end

    if nargin >=3 && ~isempty(im_basis)
        if ~isa(im_basis, 'cvx')
            error("Expected CVX vector for imaginary basis input.");
        end                
        cvx_expr = obj.Apply(real_basis, im_basis);
    else
        cvx_expr = obj.Apply(real_basis);
    end              
end