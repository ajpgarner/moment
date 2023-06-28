   function [output, negated, hash] = Conjugate(obj, input)
 % CONJUGATE Get canonical form of operator sequence's conjugate.
 %
 % SYNTAX
 %      1. output = setting.Simplify(input_str)
 %      2. [output, negated, hash] = setting.Simplify(input_str)
 %
 % The optional hash is the shortlex hash associated with this 
 % context.
 %
     arguments
         obj(1,1) MTKScenario
         input
     end

     assert(isnumeric(input) || iscell(input));
     
     [output, negated, hash] = mtk('conjugate', obj.System.RefId, input);
 end