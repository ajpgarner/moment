function hash = shortlex_hash(op_count, sequence)
% SHORTLEX_HASH Calculate shortlex hash of an operator sequence.
%
% PARAMS
%   op_count - The number of operators (including complex conjugates)
%   sequence - The operator string to hash
    arguments
        op_count (1,1) uint64
        sequence (1,:) uint64
    end
    
    hash = 1;
    stride = uint64(1);
    for index = length(sequence):-1:1
        hash = hash + stride * sequence(index);
        stride = uint64(stride * ...
                        op_count);
    end
end
