 function val = createSolvedScenario(obj, a, b)            
% CREATESOLVEDSCENARIO Bind numeric values from an SDP solve.
% 
% PARAMS
%   a - Vector of values for real basis elements
%   b (optional) - Vector of values for imaginary basis elements
%
% RETURNS
%   SOLVEDSCENARIO.SOLVEDSCENARIO or specialized subclass thereof.
%

    val = SolvedScenario.SolvedScenario(obj, a, b); 
end