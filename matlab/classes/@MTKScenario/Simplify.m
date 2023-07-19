function [output, negated, hash] = Simplify(obj, input)
 % SIMPLIFY Get canonical form of operator sequence input.
 % All applicable re-write rules will be applied.
 %
 % SYNTAX
 %      1. [output, negated, hash] = setting.Simplify(input_str)
 %      2. [o., n., h.] = setting.Simplify({cell of input_strs})
 %
 % The hash is the shortlex hash associated with this context.
 % In syntax 1, output is a uint64 row-vector, negated is scalar bool and 
 % hash is scalar uint64.
 % In syntax 2, output will be a cell array of uint64 row-vectors, and 
 % negated and hash will be arrays of matching dimensions.
 %
    assert(nargin>=2); 
    assert(isnumeric(input) || iscell(input));

    [output, negated, hash] = mtk('simplify', obj.System.RefId, input);
 end