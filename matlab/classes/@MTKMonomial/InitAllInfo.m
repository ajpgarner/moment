function obj = InitAllInfo(setting, operators, coefs, hashes, ...
                           symbols, conj, real_index, im_index, ...
                           is_aliased)
% INITALLINFO Directly initialize a MTKMonomial with supplied parameters.
% No simplifications or other checks are performed - so this function
% should only be used to construct Monomials already in canonical form.
% If the above does not make sense, do not use this function.
%

    % Special case: empty
    if isempty(coefs)
        obj = MTKMonomial.empty(size(coefs));
        return;
    end
    
    % If symbolic information not provided, partial init...
    if nargin <= 4
        obj = MTKMonomial.InitDirect(setting, operators, coefs, hashes);
        return
    end
    
    % Is alias info provided?
    if nargin < 9
        is_aliased = false(size(coefs));
    end

    % Normal case:
    dimensions = size(operators);
    obj = MTKMonomial(setting, 'overwrite', dimensions);
    if obj.IsScalar && iscell(operators)
        obj.Operators = operators{1};
    else
        obj.Operators = operators;
    end
    obj.Coefficient = coefs;
    obj.Hash = hashes;
    
    obj.symbol_id = symbols;
    obj.symbol_conjugated = conj;
    obj.re_basis_index = real_index;
    obj.im_basis_index = im_index;    
    obj.is_alias = is_aliased;
    
end