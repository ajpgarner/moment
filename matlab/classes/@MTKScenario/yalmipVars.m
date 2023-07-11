function varargout = yalmipVars(obj)
%YALMIPVARS (Forward) Get yalmip sdpvars
%
% Syntax 
%   1. real = scenario.yalmipVars();
%   2. [real, imaginary] = scenario.yalmipVars();
%
% See also: MTKMATRIXSYSTEM.CVXVARS

    [varargout{1:nargout}] = obj.System.yalmipVars();    
end

