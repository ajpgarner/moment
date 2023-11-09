   function [output, sign, hash] = Conjugate(obj, input)
 % CONJUGATE Get canonical form of operator sequence's conjugate.
 %
 % SYNTAX
 %      1. output = setting.Conjugate(input_str)
 %      2. [output, negated, hash] = setting.Conjugate(input_str)
 %
 % The sign is +1, +i, -1 or -i.
 %
 % The optional hash is the shortlex hash associated with this context.
 %
     assert(isnumeric(input) || iscell(input));     
     [output, sign, hash] = mtk('conjugate', obj.System.RefId, input);
 end