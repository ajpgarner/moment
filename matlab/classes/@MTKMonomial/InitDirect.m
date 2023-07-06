function obj = InitDirect(setting, operators, coefs, hashes)
% INITDIRECT Directly initialize a MTKMonomial with supplied parameters.
% No simplifications or other checks are performed - so this function
% should only be used to construct Monomials already in canonical form.
% If the above does not make sense, do not use this function. 
%
    dimensions = size(operators);    
    obj = MTKMonomial(setting, 'overwrite', dimensions);
    if obj.IsScalar && iscell(operators)
        obj.Operators = operators{1};
    else
        obj.Operators = operators;
    end
    obj.Coefficient = coefs;
    obj.Hash = hashes;
     
end