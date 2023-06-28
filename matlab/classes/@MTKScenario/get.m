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
    arguments
        obj (1,1) MTKScenario
        operators (1,:) uint64
    end

    item = MTKMonomial(obj, operators, 1.0);
end