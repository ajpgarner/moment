function val = MomentRulebook(obj, label, varargin)
% MOMENTRULEBOOK Creates a blank new rulebook for scenario.
%
% SYNTAX:
%   1. book = scenario.MomentRuleBook()
%   2. book = scenario.MomentRuleBook("Readable name");
%   3. book = scenario.MomentRuleBook("Readable name", [initial rules]);
%
% See also: MTKMOMENTRULEBOOK
 
   if nargin < 2
       val = MTKMomentRulebook(obj);
   else
       val = MTKMomentRulebook(obj, label, varargin{:});
   end

end