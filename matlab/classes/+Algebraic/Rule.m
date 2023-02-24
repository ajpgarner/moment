classdef Rule
    %RULE Rewrite rule for algebraic system
    
    properties(GetAccess = public, SetAccess = protected)
        LHS
        RHS
        Negated = false
    end
    
    methods
        function obj = Rule(lhs, rhs, negate)
            arguments
                lhs (1,:)
                rhs (1,:)
                negate (1,1) logical = false
            end
            
            if nargin <= 2
                negate = false;
            end
            
            % Read LHS
            if isnumeric(lhs)
                lhs = uint64(lhs);
            elseif ~isstring(lhs) && ~ischar(lhs)
                error("LHS of rule should be numeric, or string of operators.");
            end
            
            % Read RHS
            if isnumeric(rhs)
                rhs = uint64(rhs);
            elseif ~isstring(rhs) && ~ischar(names_hint)
                error("RHS of rule should be numeric, or string of operators.");
            end
            
            obj.LHS = lhs;
            obj.RHS = rhs;
            obj.Negated = logical(negate);
        end
    end
end

