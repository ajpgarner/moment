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
            
            % Check constituents all belong to correct scenario
            for other = constituents
                obj.checkSameScenario(other);
            end                        
            obj.Constituents = constituents;
            obj.orderAndMerge();
        end
        
        function val = get.IsZero(obj)
            val = isempty(obj.Constituents);
        end
    end
    
    %% Localizing matrix
    methods
        function val = LocalizingMatrix(obj, level)
        % LOCALIZINGMATRIX Create a localizing matrix for this expression.
        %
        % PARAMS
        %   level - The level of matrix to generate. 
        %
        % RETURNS
        %   A new OpMatrix.CompositeOperatorMatrix object, containing one
        %   OpMatrix.LocalizingMatrix for each constituent term of the
        %   monomial, weighted by the appropriate co-efficient.
        %
        % See also: OpMatrix.CompositeOperatorMatrix,
        %           OpMatrix.LocalizingMatrix
        %
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
        %
        % SYNTAX:
        %   1. result = poly == double
        %   2. result = double == poly
        %   3. result = poly == mono
        %   4. result = mono == poly
        %   5. result = poly == poly
        %
        % RETURNS
        %   True, if objects are functionally the same, false otherwise.
        %
        % Syntaxes 3, 4 and 5 will error if objects do not share the same
        % setting.
        %        
        % For syntaxes 1 and 2, truth requires either the polynomial have 
        % one constituent, and this to be equal to the double (see
        % Algebraic.Monomial.EQ), or for the polynomial to be zero as a
        % whole, and the double to also be zero.
        %
        % For syntaxes 3 and 4, truth requires the polynomial contain only 
        % one constituent, and that this is equal to the monomial.
        %
        % For syntax 5, truth requires that the two polynomials contain the
        % same number of terms, and these terms are all equivalent.
        %
            
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
                % Objects in different scenarios cannot be equal
                if this.Scenario ~= other.Scenario
                    val = false;
                    return;
                end
                
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
                % Objects in different scenarios cannot be equal
                if this.Scenario ~= other.Scenario
                    val = false;
                    return;
                end
                                
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
        % NE Compare LHS and RHS for value-wise inequality.
        % Logical negation of eq(lhs, rhs)
        %
        % See also: POLYNOMIAL.EQ
        %
            val = ~eq(lhs, rhs);
        end
  
    end
    
    %% Sums and multiplication
    methods        
        function val = mtimes(lhs, rhs)
       % MTIMES Multiplication.
       %
       % SYNTAX
       %   1. v = polynomial * double
       %   2. v = double * polynomial 
       %   3. v = polynomial * monomial
       %   4. v = monomial * polynomial 
       %   5. v = polynomial_A * polynomial_B
       %
       % RETURNS
       %   A new Algebraic.Polynomial with appropriate coefficients and
       %   operators.
       %
       % See also: Monomial.mtimes
       %
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
                this.checkSameScenario(other);                
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
                this.checkSameScenario(other);
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
        % PLUS Addition.
        %
        % SYNTAX:
        %   1. val = poly + double
        %   2. val = double + poly
        %   3. val = mono + poly
        %   4. val = poly + mono
        %   5. val = poly + poly
        %
        % RETURNS 
        %   Either 0, an Algebraic.Monomial, or Algebraic.Polynomial.
        %   Numeric 0 is returned if all terms cancel out after addition.
        %   Algebraic.Monomial is returned if all but one term cancels out.
        %   Otherwise, Algebraic.Polynomial is returned.
        %
        % See also: ALGEBRAIC.MONOMIAL.PLUS
        %
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
                if ~isscalar(other)
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
            this.checkSameScenario(other);
                        
            % Add monomial to polynomial?
            if isa(other, 'Algebraic.Monomial')
                components = horzcat(this.Constituents, other);
                val = Algebraic.Polynomial(this.Scenario, components);
            elseif isa(other, 'Algebraic.Polynomial')
                if ~other.IsZero
                    components = horzcat(this.Constituents, other.Constituents);
                    val = Algebraic.Polynomial(this.Scenario, components);
                end
            else
                error(['Assertion failed: ',...
                       'other should be Monomial or Polynomial.']);
            end
            
            % Degrade, as necessary
            val = +val;
        end
        
        function val = uplus(obj)
        % UPLUS Unary plus.
        % If single-element polynomial, will degrade to monomial.
        % If equal to zero, will degrade to numeric 0.
            if length(obj.Constituents) == 1
                val = obj.Constituents(1);
                if abs(val.Coefficient) < 2*eps
                    val = 0;
                end
            elseif obj.IsZero
                val = 0;
            else
                val = obj;
            end
        end
        
        function val = uminus(obj)
        % UMINUS Unary minus.
        % Creates new polynomial, with all coefficients negated.
        %
            new_constituents = Algebraic.Monomial.empty(1,0);
            for c = obj.Constituents
                new_constituents(end+1) = -c;
            end
            val = Algebraic.Polynomial(obj.Scenario, new_constituents);
        end
        
        function val = minus(lhs, rhs)
        % MINUS Subtraction. Equivalent to addition of lhs with -rhs.
        %
        % See also: ALGEBRAIC.POLYNOMIAL.PLUS, ALGEBRAIC.POLYNOMIAL.UMINUS.
        %
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            val = lhs + (-rhs);
        end
                
        function val = ctranspose(obj)
        % CTRANSPOSE Complex conjugation.
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

