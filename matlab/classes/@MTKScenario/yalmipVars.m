function varargout = yalmipVars(obj,export_type)
%YALMIPVARS (Forward) Get yalmip sdpvars
%
% Syntax 
%   1. real = scenario.yalmipVars();
%   2. [real, imaginary] = scenario.yalmipVars();
%   3. imaginary = scenario.yalmipVars('imaginary');
%
% See also: MTKMATRIXSYSTEM.CVXVARS

    if nargin == 1
        [varargout{1:nargout}] = obj.System.yalmipVars();
    else
        [varargout{1:nargout}] = obj.System.yalmipVars(export_type);
    end

end

