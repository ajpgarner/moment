  function val = Solved(obj, a, b)
% SOLVED Bind numeric values from an SDP solve.
% 
% PARAMS:
%   a - Vector of coefficients for real basis elements.
%   b (optional) - Vector of coefficients for imaginary basis
%                  elements.
%
% RETURNS:
%   SOLVEDSCENARIO.SOLVEDSCENARIO or specialized subclass thereof.
%
% See also: SOLVEDSCENARIO.SOLVEDSCENARIO
%
    arguments
        obj (1,1) MTKScenario
        a (:,1) double = double.empty(0,1)
        b (:,1) double = double.empty(0,1)
    end

    if nargin <= 2
        b = double.empty(0,1);
    end

    val = obj.createSolvedScenario(a, b);            
end