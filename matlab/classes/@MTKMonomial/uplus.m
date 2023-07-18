 function val = uplus(obj)
% UPLUS Unary plus (typically does nothing, could degrade to zero).
%
% SYNTAX
%   v = +mono
%
% RETURNS
%   Either the same monomial object, or a monomial object with 
%   numbers close to zero pruned.
%
   [new_coefs, mask, mask_re, mask_im] = obj.Scenario.Prune(obj.Coefficient);
   if ~any(mask | mask_re | mask_im, 'all')
       val = obj;
   else
       if obj.IsScalar
           if mask
               new_ops = uint64.empty(1,0);
           else
               new_ops = obj.Operators;
           end
       else
           new_ops = MTKUtil.cell_mask(obj.Operators, mask, []);
       end

       val = MTKMonomial(obj.Scenario, new_ops, new_coefs);
   end
end
