classdef Zero < Abstract.ComplexObject
%ZERO Zero, as represented as a ComplexObject.
        
    methods
        function obj = Zero(setting)
            %ZERO Construct an instance of this class
            %   Detailed explanation goes here
           
            obj = obj@Abstract.ComplexObject(setting);
            obj.ObjectName = "0";
        end        
    end
    
    
    %% Virtual methods
    methods(Access=protected)
        function success = calculateCoefficients(obj)
            sys = obj.Scenario.System;
            % All zeros
            obj.real_coefs = sparse(1, double(sys.RealVarCount));
            obj.im_coefs = sparse(1, double(sys.ImaginaryVarCount));
            success = true;
        end
    end
    
    
    %% Comparison
    methods
        function val = eq(lhs, rhs)    
        % EQ Compare for equality.
        % Class is dominated, so only check vs. self and vs. doubles
        %
            if isa(lhs,'Algebraic.Zero')
                if isa(rhs,'Algebraic.Zero')
                    val = true;
                    return;
                elseif isa(rhs, 'double')
                    val = (rhs == 0);
                    return;
                end
            elseif isa(lhs, 'double')
                val = (lhs == 0);
                return;
            end
            
            % Could not compare
            error("_==_ not defined between %s and %s.",...
                  class(lhs), class(rhs));                
        end
        
        function val = ne(lhs, rhs)
        % NE Compare for inequality.
        % Class is dominated, so only check vs. self and vs. doubles
        %
            if isa(lhs, 'Algebraic.Zero')
                val = ~lhs.eq(rhs);
            else
                val = ~rhs.eq(lhs);
            end
        end
    end
    
    %% Algebraic manipulation
    methods
        function val = uplus(obj)
        % UPLUS Unary plus (does nothing).
        %
        % SYNTAX
        %   v = +mono
        %
        % RETURNS
        %   The same zero object.
        %
            val = obj;
        end
        
        function val = uminus(obj)
        % UMINUS Unary minus (does nothing).
        %
        % SYNTAX
        %   v = -mono
        %
        % RETURNS
        %   The same zero object.
        %
            val = obj;
        end
        
        function val = mtimes(lhs, rhs)
        % MTIMES Multiplication. Goes to zero in all cases.
        %
        % SYNTAX
        %   1. v = zero * zero 
        %   2. v = double * zero 
        %   3. v = zero * monomial
        %
        % RETURNS
        %   A new Algebraic.Zero.
        %
        % See also: ALGEBRAIC.MONOMIAL.MTIMES.
        % See also: ALGEBRAIC.POLYNOMIAL.MTIMES.
        %
            arguments
                lhs (1,1)
                rhs (1,1)
            end
           
            if ~isa(lhs, 'Algebraic.Zero')
                this = rhs;
            else
                this = lhs;
            end
                                    
            val = Algebraic.Zero(this.Setting);
        end
        
        function val = plus(lhs, rhs)
        % PLUS Addition.
        %
        % SYNTAX:
        %   1. val = zero + double
        %   2. val = double + zero
        %   3. val = zero + zero
        %
        % RETURNS (Syntax 1,2)
        %   Monomial proportional to identity with coefficient of double. 
        %
        % RETURNS (Suntax 3)
        %   Algebraic.Zero
        %   
        % Due to class precedence, "zero + mono" and "zero + poly" cases 
        % are handled by Algebraic.Monomial.plus and 
        % Algebraic.Polynomial.plus respectively.
        %
        % See also: ALGEBRAIC.MONOMIAL.PLUS, ALGEBRAIC.POLYNOMIAL.PLUS
        %
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Add a scalar by a built-in type?
            if ~isa(lhs, 'Algebraic.Zero')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            if isnumeric(other)
                if other == 0
                    val = Algebraic.Zero(this.Scenario);
                else
                    val = Algebraic.Monomial(this.Scenario, ...
                                             uint64.empty(1,0), other);
                end
                return;
            elseif isa(lhs, 'Algebraic.Zero')
                val = Algebraic.Zero(this.Scenario);
                return;
            end
            
            % Could not compare
            error("_+_ not defined between %s and %s.",...
                  class(lhs), class(rhs));     
            
        end
        
        function val = minus(lhs, rhs)
        % MINUS Subtraction.
        %
        % SYNTAX:
        %   1. val = zero - double
        %   2. val = double - zero
        %   3. val = zero - zero
        %
        % RETURNS (Syntax 1,2)
        %   Monomial proportional to identity with coefficient of double. 
        %
        % RETURNS (Suntax 3)
        %   Algebraic.Zero
        %   
        % Due to class precedence, "zero - mono" and "zero - poly" cases 
        % are handled by Algebraic.Monomial.plus and 
        % Algebraic.Polynomial.plus respectively.
        %
        % See also: ALGEBRAIC.MONOMIAL.PLUS, ALGEBRAIC.POLYNOMIAL.PLUS
        %
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            val = lhs + (-rhs);            
        end
    end
    
        
    %% Apply substitution rules
    methods        
        function val = ApplyRules(obj, rulebook)
        % APPLYRULES Transform moments of matrix according to rulebook.
        %
        % Effectively applies rules to each constituent matrix in turn.
        % 
            arguments
                obj (1,1) Algebraic.Zero
                rulebook (1,1) MomentRuleBook
            end
            
            val = Algebraic.Zero(obj.Scenario);
        end
    end   
end

