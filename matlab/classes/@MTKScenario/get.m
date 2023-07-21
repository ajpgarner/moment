  function item = get(obj, operators)
% GET Return a monomial object associated with an operator string.
% Essentially, forward to MTKMonomial's constructor.
%
% PARAMS:
%     operators - The string of operators.
%
% RETURNS:
%     Object of type SYMBOLIC.MONOMIAL representing the string.
%
% See also: SYMBOLIC.MONOMIAL
%

    if nargin < 2
        error("Must specify what to get!");
    end
    
    if isnumeric(operators)  
        item = MTKMonomial(obj, operators, 1.0);
        return;
    end
    
    if iscell(operators)
        item = MTKMonomial(obj, operators, 1.0);
        return
    end
    
    error("Do not know how to get object by index of type '%s'", class(operators));
end