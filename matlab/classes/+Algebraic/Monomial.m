classdef Monomial < Abstract.ComplexObject
    %MONOMIAL A monomial expression, as part of an algebraic setting.
        
    properties(GetAccess = public, SetAccess = protected)
        Operators % The operator string defining this monomial.        
        Coefficient % Scalar co-efficient factor of the monomial.
        Hash % Hash of the operator sequence in this monomial.
    end
    
    properties(Dependent, GetAccess = public)
        % String representation of operators.
        OperatorString 

        % True if monomial can be found in symbol table.
        FoundSymbol  
        
        % The ID number of the corresponding symbol in table.
        SymbolId     
        
        % True if this monomial represents a conjugation of the symbol in the table.
        SymbolConjugated 
        
        % The real basis element representing the real part of this monomial, or 0.
        RealBasisIndex 
        
        % The imaginary basis element representing the imaginary part of this monomial, or 0.
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
        % MONOMIAL Construct a monomial.
        %
        % PARAMS
        %   setting - The algebraic scenario the monomial is a part of.
        %   operators - The operator string describing the monomial.
        %   scale - The scalar coefficient premultiplying the monomial.
        %
            arguments
                setting (1,1) AlgebraicScenario
                operators (1,:) 
                scale (1,1) double = 1.0
            end
            
            obj = obj@Abstract.ComplexObject(setting);
   
            [obj.Operators, obj.Hash] = setting.Simplify(operators);
            obj.Coefficient = scale;
           
        end        
    end
    
    %% Localizing matrix...
    methods
        function val = LocalizingMatrix(obj, level)
        % LOCALIZINGMATRIX Create a localizing matrix for this expression.
        %
        % PARAMS
        %   level - The level of matrix to generate. 
        %           Set to 0 for a 1x1 matrix containing just the monomial 
        %           expression.
        %
        % RETURNS
        %   A new OpMatrix.CompositeOperatorMatrix object, containing the
        %   localizing matrix associated with this monomial.
        %
        % See also: OpMatrix.CompositeOperatorMatrix,
        %           OpMatrix.LocalizingMatrix
        %
            arguments
                obj (1,1) Algebraic.Monomial
                level (1,1) uint64
            end
            lm = obj.RawLocalizingMatrix(level);
            val = OpMatrix.CompositeOperatorMatrix(lm, obj.Coefficient);
        end
        
        function val = RawLocalizingMatrix(obj, level)
        % RAWLOCALIZINGMATRIX Create a normalized localizing matrix for this expression.
        %
        % The created matrix ignores the coefficient associated with the
        % monomial (i.e. just creating a LM for the operator 'word'). To 
        % construct the full localizing matrix taking into account the
        % co-efficient, use Monomial.LocalizingMatrix instead.
        %
        % PARAMS
        %   level - The level of matrix to generate. 
        %           Set to 0 for a 1x1 matrix containing just the monomial 
        %           expression.
        %
        % RETURNS
        %   A new OpMatrix.LocalizingMatrix object.
        %
        % See also: Monomial.LocalizingMatrix, OpMatrix.LocalizingMatrix
        %
            arguments
                obj (1,1) Algebraic.Monomial
                level (1,1) uint64
            end
            val = OpMatrix.LocalizingMatrix(obj.Scenario, ...
                obj.Operators, level);
        end
    end
    
    %% Accessors: Operator name
    methods
        function val = get.OperatorString(obj)
            if ~isempty(obj.Operators)
                as_str = obj.Scenario.RuleBook.ToStringArray(obj.Operators);
                val = join(as_str, ' ');
            else
                val = "I";
            end
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
        %
        % SYNTAX
        %   1. result = mono_A == mono_B
        %   2. result = mono == double
        %   3. result = double == mono
        %
        % RETURNS
        %   True if objects are functionally the same, false otherwise.
        %
        % Syntax 1 will throw an exception if objects are not part of the
        % same scenario.
        %
        % For syntax 1, it is sufficient for equality that the monomials 
        % have the same coefficient and operator strings.
        %
        % For syntaxes 2 and 3, truth requires either the operator string 
        % of mono to be empty and the coefficient to match the double; or 
        % for the double to be zero and the monomial's coefficient to also 
        % be zero.
        %
        % Due to class precedence, "mono == poly" and "poly == mono" are 
        % handled by Algebraic.Polynomial.eq.
        %
        % See also: ALGEBRAIC.POLYNOMIAL.EQ
        %
            
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
            
            % Compare vs. another monomial
            if isa(other, 'Algebraic.Monomial')                
                % Not equal if scenarios do not match
                if this.Scenario ~= other.Scenario
                    val = false;
                    return
                end
                
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
        % NE Compare LHS and RHS for value-wise inequality.
        % Logical negation of eq(lhs, rhs)
        %
        % See also: MONOMIAL.EQ
        %
            val = ~eq(lhs, rhs);
        end
    end
    
    %% Algebraic manipulation
    methods
        function val = uplus(obj)
        % UPLUS Unary plus (do nothing).
        %
        % SYNTAX
        %   v = +mono
        %
        % RETURNS
        %   The same monomial object.
        %
            val = obj;
        end
        
        function val = uminus(this)
        % UMINUS Unary minus; inverts coefficient sign.
        %
        % SYNTAX
        %   v = -mono
        %
        % RETURNS
        %   A new Algebraic.Monomial, with the coefficient negated.
        %
            val = Algebraic.Monomial(this.Scenario, this.Operators, ...
                double(-this.Coefficient));
        end
        
        function val = mtimes(lhs, rhs)
        % MTIMES Multiplication.
        %
        % SYNTAX
        %   1. v = monomial * double
        %   2. v = double * monomial
        %   3. v = monomial * monomial
        %
        % RETURNS
        %   A new Algebraic.Monomoial with appropriate coefficients and
        %   operators; except when double = 0, then 0.
        %
        % Due to class precedence, poly * mono and mono * poly cases are 
        % handled by Algebraic.Polynomial.mtimes.
        %
        % See also: ALGEBRAIC.POLYNOMIAL.MTIMES.
        %
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
                
                if abs(other) < eps(1)
                    val = 0;
                    return;
                end
                
                
                val = Algebraic.Monomial(this.Scenario, this.Operators, ...
                    double(this.Coefficient * other));
            elseif isa(other, 'Algebraic.Monomial') % mono * mono = mono
                this.checkSameScenario(other);
                
                val = Algebraic.Monomial(this.Scenario, ...
                    [this.Operators, other.Operators], ...
                    double(this.Coefficient * other.Coefficient));
            else
                error("_*_ not defined between " + class(lhs) ...
                    + " and " + class(rhs));
            end
        end
        
        function val = plus(lhs, rhs)
        % PLUS Addition.
        %
        % SYNTAX:
        %   1. val = mono + double
        %   2. val = double + mono
        %   3. val = mono_A + mono_B
        %
        % RETURNS (Syntax 1,2)
        %   0 if monomial is proportional to identity with coefficient of
        %   -double. Algebraic.Monomial, if monomial is proportional to 
        %   identity but not equal to double. Algebraic.Polynomial with two 
        %   terms otherwise.
        %
        % RETURNS (Suntax 3)
        %   Algebraic.Monomial if operator string in two monomials matches,
        %   and the coefficients are not negations of each other. 0 if the
        %   operator string of the two monomials matches, and the
        %   coefficients negate each other. Algebraic.Polynomial with two 
        %   terms otherwise.
        %   
        % Due to class precedence, "poly + mono" and "mono + poly" cases 
        % are handled by Algebraic.Polynomial.plus.
        %
        % See also: ALGEBRAIC.POLYNOMIAL.PLUS
        %
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            % Is it simply two monomials of the same type?
            if isa(lhs, 'Algebraic.Monomial') ...
                    && isa(rhs, 'Algebraic.Monomial')
                
                lhs.checkSameScenario(rhs);
                
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
            
            % Quick pass through for "+0" case
            if Util.is_scalar_zero(other)
                val = this;
                return;
            end
            
            % Otherwise, promote to polynomial to handle.
            this = Algebraic.Polynomial(this.Scenario, [this]);
            
            % Add commutes, so ordering does not matter.
            val = this.plus(other);
        end
        
        
        function val = minus(lhs, rhs)
        % MINUS Subtraction, defined as addition of lhs with -rhs.
        %
        % See also: ALGEBRAIC.MONOMIAL.PLUS, ALGEBRAIC.MONOMIAL.UMINUS
        %
            arguments
                lhs (1,1)
                rhs (1,1)
            end
            
            val = lhs + -rhs;
        end
        
        function val = ctranspose(obj)
        % CTRANSPOSE Complex conjugation.
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
