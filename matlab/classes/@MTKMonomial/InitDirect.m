function obj = InitDirect(setting, operators, coefs, hashes)
% INITDIRECT Directly initialize a MTKMonomial with supplied parameters.
% No simplifications or other checks are performed - so this function
% should only be used to construct Monomials already in canonical form.
% If the above does not make sense, do not use this function. 
%

    % Special case: empty
    if isempty(coefs)
        obj = MTKMonomial.empty(size(coefs));
        return;
    end

    % Normal case
    dimensions = size(coefs);
    obj = MTKMonomial(setting, 'overwrite', dimensions);
    if obj.IsScalar && iscell(operators)
        obj.Operators = operators{1};
    else
        obj.Operators = operators;
    end
    obj.Coefficient = coefs;
    obj.Hash = hashes;
     
end