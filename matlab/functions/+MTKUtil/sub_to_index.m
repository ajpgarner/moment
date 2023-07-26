function indices = sub_to_index(sz, subs)
%SUB_TO_INDEX Similar to sub2ind, but with distributive behaviour.
%
% SYNTAX
%   1. indices = sub_to_index([size array], [i1, i2, ... iN])
%   2. indices = sub_to_index([size array], {[i1... iN], [j1... jM]})
%
% Syntax 1 trivially echos i1, ... iN.
% Syntax 2 produces N*M outputs, listing each offset in (col-major) order
%          specified.
%
% Example:
%       x = sub_to_index([2 2], {[2], [1 2]})
% output: [2 4]
%
    assert(nargin == 2);    
    indices = mtk('flatten_indices', sz, subs);    
end

