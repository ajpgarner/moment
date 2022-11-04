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
                lhs (1,:) uint64
                rhs (1,:) uint64
                negate (1,1) logical = false
            end
            
            % Determine whether lhs or rhs is longer...
            lhs_longer = true;
            if length(rhs) > length(lhs)
                % RHS is longer, so swap ordering
                lhs_longer = false;
            elseif length(rhs) == length(lhs)
                % Same length, so test lexico ordering
                [~, i] = sortrows([lhs; rhs]);
                lhs_longer = i(1) == 2;
            end

            % Store in canonical order
            if lhs_longer
                obj.LHS = lhs;
                obj.RHS = rhs;
            else
                obj.LHS = rhs;
                obj.RHS = lhs;
            end
            
            % Store whether rule has a minus sign            
            obj.Negated = negate;
        end
    end
end

