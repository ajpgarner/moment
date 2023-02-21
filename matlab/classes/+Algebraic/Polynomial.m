classdef (InferiorClasses={?Algebraic.Monomial}) Polynomial < Abstract.ComplexObject
    %POLYNOMIAL
    
    properties
        Constituents = Algebraic.Monomial.empty(1,0)
    end
    
    properties(Dependent)
        IsZero
    end
    
    methods(Static)
        function obj = Zero(setting)
            arguments
                setting (1,1) Abstract.Scenario
            end
            obj = Algebraic.Polynomial(setting, ...
                                       Algebraic.Monomial.empty(1,0));
        end
    end
    
    methods
        function obj = Polynomial(setting, constituents)
            arguments
                setting (1,1) Abstract.Scenario
                constituents (1,:) Algebraic.Monomial
            end
            obj = obj@Abstract.ComplexObject(setting);
            obj.Constituents = constituents;
            obj.orderAndMerge();
        end
        
        function val = get.IsZero(obj)
            val = isempty(obj.Constituents);
        end
    end
    
    %% Localizing matrix...
    methods
        function val = LocalizingMatrix(obj, level)
            arguments
                obj (1,1) Algebraic.Polynomial
                level (1,1) uint64
            end
            lm = OpMatrix.LocalizingMatrix.empty(1,0);
            w = double.empty(1,0);
            
            for c = obj.Constituents
                lm(end+1) = c.RawLocalizingMatrix(level);
                w(end+1) = c.Coefficient;
            end
            
            val = OpMatrix.CompositeOperatorMatrix(lm, w);
        end
    end
    
    %% Comparison
    methods
        function val = eq(lhs, rhs)
        % EQ Compare LHS and RHS for value-wise equality.
            
            % Trivially equal if same object
            if eq@handle(lhs, rhs)
                val = true;
                return;
            end
       
            if isa(lhs, 'Algebraic.Polynomial')
                this = lhs;
                other = rhs;
            else
                this = rhs;
                other = lhs;
            end
            
            tol = 2*eps(1);
            
            if isnumeric(other)
                if length(other) ~= 1
                    error("_==_ only supported for scalar comparison.");
                end
                
                % Polynomial is zero, RHS is zero
                if this.IsZero && abs(other) < tol
                    val = true;
                    return;
                end
                % Promoted monomial, compare as if monomial
                if length(this.Constituents) == 1
                    as_mono = this.Constituents(1);
                    val = as_mono.eq(other);
                    return;
                end
                % Wrong number of elements, mismatch
                val = false;
                return;                    
            end
            
            if isa(other, 'Algebraic.Monomial')
                % If length one, compare as Monomial
                if length(this.Constituents) == 1
                    as_mono = this.Constituents(1);
                    val = as_mono.eq(other);
                    return;
                end
                % Wrong number of elements, mismatch
                val = false;
                return;    
            end
            
            if isa(other, 'Algebraic.Polynomial')
                % Wrong number of elements, mismatch
                if length(this.Constituents) ~= length(other.Constituents)
                    val = false;
                    return;
                end
                
                % If any constituent part does not match, mismatch
                for i = 1:length(this.Constituents)
                    if this.Constituents(i) ~= other.Constituents(i)
                        val = false;
                        return;
                    end
                end
                val = true;
                return;               
            end
            

            error("_==_ not defined between " + class(lhs) ...
                    + " and " + class(rhs));
        end 
        
        function val = ne(lhs, rhs)
            % NEQ Compare LHS and RHS for value-wise inequality.
            val = ~eq(lhs, rhs);
        end
  
    end
    
    %% Sums and multiplication
    methods        
        function val = mtimes(lhs, rhs)
        % MTIMES Multiplication
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Pre-multiplication by a built-in type
            if ~isa(lhs, 'Algebraic.Polynomial')
                this = rhs;
                other = lhs;
                premult = true;
            else
                this = lhs;
                other = rhs;
                premult = false;
            end
            
            if isnumeric(other) % Scalar multiplication by number...
                if length(other) ~= 1
                    error("_*_ only supported for scalar multiplication.");
                end
                
                new_coefs = Algebraic.Monomial.empty(1,0);
                for i = 1:length(this.Constituents)
                    old_m = this.Constituents(i);
                    new_coefs(end+1) = ...
                        Algebraic.Monomial(this.Scenario, ...
                                           old_m.Operators, ...
                                           double(old_m.Coefficient * other));
                end
                val = Algebraic.Polynomial(this.Scenario, new_coefs);
            elseif isa(other, 'Algebraic.Monomial')
                new_coefs = Algebraic.Monomial.empty(1,0);
                for i = 1:length(this.Constituents)
                    old_m = this.Constituents(i);
                    if premult
                        new_ops = [other.Operators, old_m.Operators];
                    else
                        new_ops = [old_m.Operators, other.Operators];
                    end
                    new_coefs(end+1) = Algebraic.Monomial(this.Scenario, ...
                        new_ops, ...
                        double(old_m.Coefficient * other.Coefficient));                    
                end
                val = Algebraic.Polynomial(this.Scenario, new_coefs);
            elseif isa(other, 'Algebraic.Polynomial')
                val = Algebraic.Polynomial.Zero(this.Scenario);                
                for i = 1:length(this.Constituents)
                    new_m = this.Constituents(i) * other;
                    val = val + new_m;
                end
            else
                error("_*_ not defined between " + class(lhs) ...
                    + " and " + class(rhs));
            end            
        end
        
        function val = plus(lhs, rhs)
        % PLUS Addition
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Which are we??
            if ~isa(lhs, 'Algebraic.Polynomial')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            % Is other side built-in numeric; if so, cast to monomial
            if isnumeric(other)
                if length(other) ~= 1
                    error("_+_ only supported for scalar addition.");
                end
                
                % If +0; just return self (in canonical form)
                if abs(other) < 2*eps
                    val = +this;
                    return
                end
                
                
                other = Algebraic.Monomial(this.Scenario, [], double(other));
            elseif ~isa(other, 'Algebraic.Monomial') && ...
                    ~isa(other, 'Algebraic.Polynomial')
                error("_+_ not defined between " + class(lhs) ...
                        + " and " + class(rhs));
            end
            
            % Check objects are from same scenario
            if (this.Scenario ~= other.Scenario)
                error(this.err_mismatched_scenario);
            end
                        
            % Add monomial to polynomial?
            if isa(other, 'Algebraic.Monomial')
                components = horzcat(this.Constituents, other);
                val = Algebraic.Polynomial(this.Scenario, components);
            elseif isa(other, 'Algebraic.Polynomial')
                components = horzcat(this.Constituents, other.Constituents);
                val = Algebraic.Polynomial(this.Scenario, components);
            else
                error(['Assertion failed: ',...
                       'other should be Monomial or Polynomial.']);
            end
            
            % Degrade to zero, if zero
            if val.IsZero
                val = 0;
                return;
            end
            
            % Degrade to monomial, if single element
            if length(val.Constituents) == 1
                val = val.Constituents(1);
            end
            

            
        end
        
        function val = uplus(obj)
        % UPLUS Unary plus.
        % If single-element polynomial, will degrade to monomial.
        % If equal to zero, will degrade to numeric 0.
            if length(obj.Constituents) == 1
                val = obj.Constituents(1);
            elseif obj.IsZero
                val = 0;
            else
                val = obj;
            end
        end
        
        function val = uminus(obj)
        % UMINUS Unary minus
            new_constituents = Algebraic.Monomial.empty(1,0);
            for c = obj.Constituents
                new_constituents(end+1) = -c;
            end
            val = Algebraic.Polynomial(obj.Scenario, new_constituents);
        end
        
        function val = minus(lhs, rhs)
        % MINUS Subtraction
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            val = lhs + (-rhs);
        end
        
        function val = ctranspose(obj)
        % CTRANSPOSE Complex conjugation
            new_constituents = Algebraic.Monomial.empty(1,0);
            for c = obj.Constituents
                new_constituents(end+1) = c';
            end
            val = Algebraic.Polynomial(obj.Scenario, new_constituents);
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function success = calculateCoefficients(obj)
            % Early exit if we can't get symbol information for all parts...
            if ~all([obj.Constituents.FoundSymbol])
                success = false;
                return;
            end
            sys = obj.Scenario.System;
            
            % Real co-efficients
            obj.real_coefs = sparse(1, double(sys.RealVarCount));
            obj.im_coefs = sparse(1, double(sys.ImaginaryVarCount));
            
            for index = 1:length(obj.Constituents)
                cObj = obj.Constituents(index);
                cObj.refreshCoefficients();
                obj.real_coefs = obj.real_coefs + cObj.real_coefs;
                obj.im_coefs = obj.im_coefs + cObj.im_coefs;
            end
            success = true;
        end
    end
    
    
    %% Internal methods
    methods(Access=private)
        function orderAndMerge(obj)
            % Order
            [~, order] = sort([obj.Constituents.Hash]);
            write_index = 0;
            
            % Merge
            nc = Algebraic.Monomial.empty(1,0);
            last_hash = 0;
            for i = 1:length(obj.Constituents)
                cObj = obj.Constituents(order(i));
                if last_hash == cObj.Hash
                    ncoef = nc(write_index).Coefficient + cObj.Coefficient;
                    nc(write_index) = ...
                        Algebraic.Monomial(obj.Scenario, ...
                        cObj.Operators, ...
                        ncoef);
                else
                    write_index = write_index + 1;
                    nc(end+1) = Algebraic.Monomial(obj.Scenario, ...
                        cObj.Operators, ...
                        cObj.Coefficient);
                end
                last_hash = cObj.Hash;
            end
            
            % Trim zeros
            nz_mask = abs([nc(:).Coefficient]) >= 2*eps(0);
            obj.Constituents = nc(nz_mask);
            
            
        end
    end
end

