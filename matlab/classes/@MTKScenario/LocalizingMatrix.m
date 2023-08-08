function val = LocalizingMatrix(obj, expression, level)
%LOCALIZINGMATRIX Summary of this function goes here
%   Detailed explanation goes here
    assert(nargin==3, ...
           "A localizing matrix requires an expression and level.");
    
    if ~isa(expression, 'MTKMonomial') && ~isa(expression, 'MTKPolynomial')
        error("A localizing matrix can only be formed from a polynomial or a monomial.");
    end
    assert(expression.Scenario == obj, ...
           "The expression must be from this scenario.");
    
    val = expression.LocalizingMatrix(uint64(level));    
end

