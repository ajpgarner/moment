classdef Rule < matlab.mixin.CustomDisplay
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
            elseif ~isstring(rhs) && ~ischar(rhs)
                error("RHS of rule should be numeric, or string of operators.");
            end
            
            obj.LHS = lhs;
            obj.RHS = rhs;
            obj.Negated = logical(negate);
        end        
    end
    
    %% Display overloads
    methods(Access=protected)
        function displayScalarObject(obj)          
            header = matlab.mixin.CustomDisplay.getClassNameForHeader(obj);
            text = obj.ruleText();
            if obj.Negated
                 fprintf("%s: %s\n\n", header, text)
            else
                fprintf("%s: %s\n\n", header, text)
            end
        end
        
        function displayNonScalarObject(obj)
           dim_str = matlab.mixin.CustomDisplay.convertDimensionsToString(obj);
           name = matlab.mixin.CustomDisplay.getClassNameForHeader(obj);
           fprintf("%s %s array with rules:\n\n", dim_str, name);
           for i = 1:length(obj)
               fprintf("\t%5d:\t%s\n", i, obj(i).ruleText);
           end
           fprintf("\n");
        end
    end
    
    methods(Access=private)
        function str = ruleText(obj)
            lhs_str = Algebraic.Rule.opSeqToString(obj.LHS);
            rhs_str = Algebraic.Rule.opSeqToString(obj.RHS);
            
            str = lhs_str + "  ->  ";
            if obj.Negated
                str = str + "-";
            end
            str = str + rhs_str;
        end
    end
    
    methods(Static,Access=private)
      function str = opSeqToString(seq)
          if isempty(seq)
              str = "I";
          elseif isnumeric(seq)
              str = "[" + num2str(seq) + "]";
          elseif ischar(seq)
              str = string(seq);
          elseif isstring(seq)
              str = join(seq, " ");
          else
              str = '[UNKNOWN]';
          end
        end
    end
end

