classdef Monomial < Abstract.ComplexObject
    %MONOMIAL A monomial expression, as part of an algebraic setting
    
    properties(GetAccess = public, SetAccess = protected)
        Operators
        Coefficient
        Hash
    end
    
    properties(Dependent, GetAccess = public)
        FoundSymbol
        SymbolId
        SymbolConjugated
        RealBasisIndex
        ImaginaryBasisIndex
    end
    
    properties(Access=private)
        symbol_id = int64(-1);
        symbol_conjugated = false;
        re_basis_index = -1;
        im_basis_index = -1;
    end
    
    methods
        function obj = Monomial(setting, operators, scale)
            arguments
                setting (1,1) AlgebraicScenario
                operators (1,:) uint64
                scale (1,1) double = 1.0
            end
            
            obj = obj@Abstract.ComplexObject(setting);
            
            % Check operator string is in range
            if any(operators > setting.EffectiveOperatorCount) ...
                    || any(operators <= 0)
                error("Operator string contains out of range index.");
            end
            
            obj.Operators = operators;
            obj.Hash = obj.calculateShortlexHash();
            obj.Coefficient = scale;
        end
        
    end
    
    %% Localizing matrix...
    methods
        function val = LocalizingMatrix(obj, level)
            arguments
                obj (1,1) Algebraic.Monomial
                level (1,1) uint64
            end
            lm = obj.RawLocalizingMatrix(level);
            val = OpMatrix.CompositeOperatorMatrix(lm, obj.Coefficient);
        end
        
        function val = RawLocalizingMatrix(obj, level)
            arguments
                obj (1,1) Algebraic.Monomial
                level (1,1) uint64
            end
            val = OpMatrix.LocalizingMatrix(obj.Scenario, ...
                obj.Operators, level);
        end
    end
    
    %% Accessors: Symbol row info
    methods
        function val = get.FoundSymbol(obj)
            if obj.symbol_id < 0
                obj.loadSymbolInfo();
            end
            val = logical(obj.symbol_id >= 0);
        end
        
        function val = get.SymbolId(obj)
            obj.loadSymbolInfoOrError();
            val = obj.symbol_id;
            
        end
        
        function val = get.SymbolConjugated(obj)
            obj.loadSymbolInfoOrError();
            val = obj.symbol_conjugated;
        end
        
        function val = get.RealBasisIndex(obj)
            obj.loadSymbolInfoOrError();
            val = obj.re_basis_index;
        end
        
        function val = get.ImaginaryBasisIndex(obj)
            obj.loadSymbolInfoOrError();
            val = obj.im_basis_index;
        end
    end
    
    %% Equality
    methods
        function val = eq(lhs, rhs)
            % EQ Compare LHS and RHS for value-wise equality.
            
            % Trivially equal if same object
            if eq@handle(lhs, rhs)
                val = true;
                return;
            end
            
            tol = 2*eps(1.0);
            
            if isa(lhs, 'Algebraic.Monomial')
                this = lhs;
                other = rhs;
            else
                this = rhs;
                other = lhs;
            end
            
            % Compare vs. a polynomial: use Algebraic.Polynomial
            if isa(other, 'Algebraic.Polynomial')
                val = other.eq(lhs);
                return;
            end
            
            % Compare vs. another monomial
            if isa(other, 'Algebraic.Monomial')
                % Equal if same operators and coefficient.
                if (isequal(this.Operators, other.Operators) && ...
                        (abs(this.Coefficient - other.Coefficient) < tol))
                    val = true;
                    return;
                end
                
                % Equal if both coefficents are 0
                if ((abs(this.Coefficient) < tol) && ...
                        (abs(this.Coefficient) < tol))
                    val = true;
                    return;
                end
                val = false;
                return;
            end
            
            % Compare vs. a number
            if isnumeric(other)
                % Must be single number
                if length(other) ~= 1
                    error("_==_ only supported for scalar comparison.");
                end
                
                
                % Equal if no operators, and coefficient matches
                if (isempty(this.Operators) && ...
                        (abs(this.Coefficient - other) < tol))
                    val = true;
                    return;
                end
                % Equal if coef. is 0, and rhs is 0
                if ((abs(this.Coefficient) < tol) && (abs(other) < tol))
                    val = true;
                    return;
                end
                val = false;
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
    
    %% Algebraic manipulation
    methods
        
        function val = uplus(obj)
            % UPLUS Unary plus (do nothing).
            val = obj;
        end
        
        function val = uminus(this)
            % UMINUS Unary minus - invert coefficient sign.
            val = Algebraic.Monomial(this.Scenario, this.Operators, ...
                double(-this.Coefficient));
        end
        
        function val = mtimes(lhs, rhs)
            % MTIMES Multiplication
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Pre-multiplication by a built-in type
            if ~isa(lhs, 'Algebraic.Monomial')
                this = rhs;
                other = lhs;
                if ~isnumeric(other)
                    error(['Pre-multiplication _*_ should only be ',...
                        'invoked when LHS is a built-in type.']);
                end
            else
                this = lhs;
                other = rhs;
            end
            
            if isnumeric(other) % Scalar number multiplication:
                if length(other) ~= 1
                    error("_*_ only supported for scalar multiplication.");
                end
                
                val = Algebraic.Monomial(this.Scenario, this.Operators, ...
                    double(this.Coefficient * other));
            elseif isa(other, 'Algebraic.Monomial') % mono * mono = mono
                val = Algebraic.Monomial(this.Scenario, ...
                    [this.Operators, other.Operators], ...
                    double(this.Coefficient * other.Coefficient));
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
            
            % Is it simply two monomials of the same type?
            if isa(lhs, 'Algebraic.Monomial') ...
                    && isa(rhs, 'Algebraic.Monomial')
                
                % Are these monomials equal up to negation?
                if eq(lhs, -rhs)
                    val = 0;
                    return;
                end
                
                % Are these monomials with the same operator string
                if isequal(lhs.Operators, rhs.Operators)
                    val = Algebraic.Monomial(lhs.Scenario, lhs.Operators,...
                        double(lhs.Coefficient + rhs.Coefficient));
                    return;
                end
            end
            
            % Add a scalar by a built-in type?
            if ~isa(lhs, 'Algebraic.Monomial')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            % Promote to polynomial
            this = Algebraic.Polynomial(this.Scenario, [this]);
            
            % Add (commutes, so ordering does not matter)
            val = this.plus(other);
        end
        
        
        function val = minus(lhs, rhs)
            % MINUS Subtraction
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            val = lhs + -rhs;
        end
        
        function val = ctranspose(obj)
            % CTRANSPOSE Complex conjugation
            conj_ops = mtk('conjugate', ...
                obj.Scenario.System.RefId, ...
                obj.Operators);
            
            val = Algebraic.Monomial(obj.Scenario, ...
                conj_ops, obj.Coefficient);
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function success = calculateCoefficients(obj)
            % Early exit if we can't get symbol information...
            obj_id = obj.SymbolId;
            if obj_id < 0
                success = false;
                return;
            end
            
            sys = obj.Scenario.System;
            
            % Real coefficients
            obj.real_coefs = sparse(1, double(sys.RealVarCount));
            if obj.re_basis_index > 0
                obj.real_coefs(obj.re_basis_index) = ...
                    obj.Coefficient;
            end
            
            % Imaginary coefficients
            obj.im_coefs = sparse(1, double(sys.ImaginaryVarCount));
            if obj.im_basis_index > 0
                if obj.SymbolConjugated
                    obj.im_coefs(obj.im_basis_index) = ...
                        -obj.Coefficient;
                else
                    obj.im_coefs(obj.im_basis_index) = ...
                        obj.Coefficient;
                end
            end
            
            success = true;
        end
    end
    
    %% Internal methods
    methods (Access=private)
        function loadSymbolInfoOrError(obj)
            % Cached value?
            if obj.symbol_id == -1
                obj.loadSymbolInfo();
                
                if obj.symbol_id == -1
                    error("No associated symbol found in matrix system.");
                end
            end
        end
        
        function val = calculateShortlexHash(obj)
            val = 1;
            stride = uint64(1);
            for index = length(obj.Operators):-1:1
                val = val + stride*obj.Operators(index);
                stride = uint64(stride * ...
                    obj.Scenario.EffectiveOperatorCount);
            end
        end
        
        function success = loadSymbolInfo(obj)
            if obj.Scenario.HasMatrixSystem
                sys = obj.Scenario.System;
                [row, conjugated] = mtk('symbol_table', ...
                    sys.RefId, ...
                    obj.Operators);
                if (isa(row, 'logical') && (row == false))
                    obj.setDefaultSymbolInfo();
                    success = false;
                else
                    obj.symbol_id = uint64(row.symbol);
                    obj.re_basis_index = uint64(row.basis_re);
                    obj.im_basis_index = uint64(row.basis_im);
                    obj.symbol_conjugated = logical(conjugated);
                    success = true;
                end
            else
                obj.setDefaultSymbolInfo();
                success = false;
            end
        end
        
        function setDefaultSymbolInfo(obj)
            obj.symbol_id = -1;
            obj.symbol_conjugated = false;
            obj.re_basis_index = 0;
            obj.im_basis_index = 0;
        end
    end
end
