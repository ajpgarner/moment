classdef OperatorRule < matlab.mixin.CustomDisplay
    %RULE Rewrite rule for operators in algebraic matrix system.
    
    properties(GetAccess = public, SetAccess = protected)
        LHS % Left-hand-side (match pattern) of rule.
        RHS % Right-hand-side (substitution) of rule.
        Negated = false % True if rule replaces LHS with -RHS.
		Imaginary = false % True if RHS should be multiplied by a multiple of i
    end
    
    properties(GetAccess = public, Dependent)
        IsZeroRHS
    end
        
    
    methods
        function obj = OperatorRule(lhs, rhs, negate, imaginary)
		
			% Validate inputs
			if nargin < 2
				error("Rules require a left and a right hand side");
			end
		            
            % Read LHS
			lhs = reshape(lhs, 1, []);
            if isnumeric(lhs)
				lhs = uint64(lhs);
            elseif ~isstring(lhs) && ~ischar(lhs)
                error("LHS of rule should be numeric, or string of operators.");
            end
            
            % Read RHS
			rhs = reshape(rhs, 1, []);
            if isnumeric(rhs)		
				rhs = uint64(rhs);
            elseif ~isstring(rhs) && ~ischar(rhs)
                error("RHS of rule should be numeric, or string of operators.");
            end
			
            if nargin < 3
                negate = false;
            end
			
			if nargin < 4
				imaginary = false;
			end
            
            obj.LHS = lhs;
            obj.RHS = rhs;
            obj.Negated = logical(negate(1));
			obj.Imaginary = logical(imaginary(1));
        end
        
        function str = string(obj)
        % STRING Convert rule to human-readable string.
            switch numel(obj)
                case 0
                    str = "No rules";
                case 1
                    str = obj.ruleText;
                otherwise
                    once = false;
                    str = "";
                    for rule = obj
                        if once
                            str = str + newline;
                        end
                        str = str + rule.ruleText;
                        once = true;
                    end
            end
        end
        
        function val= get.IsZeroRHS(obj)
            val = isequal(obj.RHS, uint64(0));
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
            lhs_str = Algebraic.OperatorRule.opSeqToString(obj.LHS);
            if obj.IsZeroRHS
                rhs_str = "0";
            else
                rhs_str = Algebraic.OperatorRule.opSeqToString(obj.RHS);
            end
            
            str = lhs_str + "  ->  ";
            if obj.Negated
                str = str + "-";
            end
			if obj.Imaginary
				str = str + "i";
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

