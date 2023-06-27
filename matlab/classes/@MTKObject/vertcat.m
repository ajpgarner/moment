function output = vertcat(varargin)
% VERTCAT Vertical concatenation [a; b; c]
%
% See also: MTKOBJECT.CAT
% 
    output = cat(1, varargin{:});
end