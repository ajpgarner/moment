function val = MomentRulebook(obj, label)
% MOMENTRULEBOOK Creates a blank new rulebook for scenario.
%
% SYNTAX:
%   1. book = scenario.MomentRuleBook()
%   2. book = scenario.MomentRuleBook("Human-readable name");
%
% See also: MTKMOMENTRULEBOOK
 
   if nargin < 2
       val = MTKMomentRulebook(obj);
   else
       val = MTKMomentRulebook(obj, label);
   end

end