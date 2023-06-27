function output = horzcat(varargin)      
% HORZCAT Horizontal concatenation [a, b, c]
%
% See also: MTKOBJECT.CAT
    output = cat(2, varargin{:});
end