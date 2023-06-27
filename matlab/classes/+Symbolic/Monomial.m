classdef Monomial < Symbolic.ComplexObject
%MONOMIAL A monomial expression of operators (or their moment).
        
    properties(GetAccess = public, SetAccess = protected)
        Operators % The operator sequence defining this monomial.        
        Coefficient % Scalar co-efficient factor of the monomial.
        Hash % Hash of the operator sequence in this monomial.
    end
    
    properties(Dependent, GetAccess = public)
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
        
        % True, if monomial represents 0
        IsZero
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
            array_creation = false;
            create_dimensions = [1, 1];
            init_for_overwrite = false;
            
            if (nargin < 1) || ~isa(setting, 'Abstract.Scenario')
                error("First argument must be a scenario.");
            end
            
            if nargin < 2
                operators = uint64.empty(1,0); % ID
                scale = 1.0; 
            else
                if (ischar(operators) || isstring(operators)) && strcmp(operators, 'overwrite')
                    init_for_overwrite = true;
                    if nargin < 3
                        error("Overwrite mode must supply dimensions as third argument.");
                    else
                        create_dimensions = double(scale);                   
                        array_creation = prod(create_dimensions) ~= 1;
                    end                    
                elseif isnumeric(operators)                   
                    operators = reshape(uint64(operators), 1, []);
                elseif iscell(operators)
                    array_creation = true;
                    create_dimensions = size(operators);
                else
                    error("Operators strings should be supplied as row vector, or cell array of row vectors.");
                end
                
                if ~init_for_overwrite
                    if nargin < 3
                        if array_creation
                            scale = ones(create_dimensions);
                        else
                            scale = 1.0;
                        end
                    else 
                        if array_creation
                            if length(scale) == 1
                                scale = ones(create_dimensions) * scale;
                            elseif ~isequal(size(operators), size(scale))
                                error("Scale dimensions must match operator specification dimensions.");
                            else
                                scale = double(scale);
                            end
                        else
                            if length(scale) ~= 1
                                error("Scale dimensions must match operator specification dimensions.");
                            end
                            scale = double(scale);
                        end
                    end
                end
            end

            % Superclass constructor
            obj = obj@Symbolic.ComplexObject(setting, create_dimensions);
            
            if array_creation
                if init_for_overwrite
                    obj.Operators = cell(create_dimensions);
                    obj.Coefficient = zeros(create_dimensions);
                    obj.Hash = uint64(zeros(create_dimensions));                    
                else
                    [obj.Operators, negated, obj.Hash] = ...
                        setting.Simplify(operators);
                    neg_mask = ones(size(negated));
                    neg_mask(negated) = -1;
                    obj.Coefficient = scale .* neg_mask;
                    obj.Hash(obj.Coefficient == 0) = 0;                    
                end
            elseif ~init_for_overwrite
                 [obj.Operators, negated, obj.Hash] = ...
                     setting.Simplify(operators);
                 obj.Coefficient = scale;
                 if negated
                     obj.Coefficient = -obj.Coefficient;
                 end
                 if obj.Coefficient == 0
                     obj.Hash = 0;
                 end
            end
            
            obj.setDefaultSymbolInfo();
        end
    end
    
    %% Named constructors
    methods(Static)
        function obj = InitForOverwrite(setting, dimensions)
            arguments
                setting (1,1) Abstract.Scenario
                dimensions (1,:) double
            end
            obj = Symbolic.Monomial(setting, 'overwrite', dimensions);
        end
        
        function obj = InitValue(setting, values)
        % INITVALUE Create monomials representing numeric values
            arguments
                setting (1,1) Abstract.Scenario
                values (:,:) double
            end
            
            if numel(values) == 1
                obj = Symbolic.Monomial(setting, [], values);
            else
                dims = num2cell(size(values));
                obj = Symbolic.Monomial(setting, repelem({[]}, dims{:}), values);
            end
        end
    end
    
    %% Convertors
    methods
        function poly = Symbolic.Polynomial(obj)
            if obj.IsScalar
                poly = Symbolic.Polynomial(obj.Scenario, obj);
            else
                poly = Symbolic.Polynomial(obj.Scenario, obj.split());
            end
        end
    end
    
    %% Localizing matrix
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
                obj (1,1) Symbolic.Monomial
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
                obj (1,1) Symbolic.Monomial
                level (1,1) uint64
            end
            val = OpMatrix.LocalizingMatrix(obj.Scenario, ...
                obj.Operators, level);
        end
    end
    
    %% Accessors: Symbol row info
    methods
        function val = get.FoundSymbol(obj)
            if any(obj.symbol_id < 0)
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
    
    %% Accessors: Zero
    methods
        function val = get.IsZero(obj)
            if obj.IsScalar
                val = isempty(obj.Operators) && (obj.Coefficient == 0);
            else
                val = cellfun(@isempty, obj.Operators) ...
                        & (obj.Coefficient == 0);
            end
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
        % handled by Symbolic.Polynomial.eq.
        %
        % See also: ALGEBRAIC.POLYNOMIAL.EQ
        %
            
            % Trivially equal if same object
            if eq@handle(lhs, rhs)
                val = true;
                return;
            end
            
            % Pre-multiplication by a built-in type
            if ~isa(lhs, 'Symbolic.Monomial') 
                this = rhs;
                other = lhs;               
            else
                this = lhs;
                other = rhs;
            end
            
            % Check dimensions
            this_size = size(this);
            other_size = size(other);            
            if ~isequal(this_size, other_size)
                if ~(this.IsScalar || numel(other) == 1)
                    error("Objects must be same size for _==_, or one must be scalar.");
                end
            end
            
            % Compare vs. numeric?
            if isnumeric(other)
                if this.IsScalar
                    if numel(other) == 1
                        if ~isempty(this.Operators)
                            val = false;
                        else
                            val = this.Scenario.IsClose(this.Coefficient, other);
                        end                            
                    else
                        if ~isempty(this.Operators)
                            val = false(size(other));                            
                        else
                            val = this.Scenario.IsClose(...
                                repmat(this.Coefficient, size(other)),...
                                other);
                        end
                    end
                else
                    if numel(other) == 1
                        val = this.Scenario.IsClose(this.Coefficient,...
                                                repmat(other, size(this)));
                        empty = cellfun(@isempty, this.Operators);
                        val(empty) = false;
                    else
                        val = this.Scenario.IsClose(this.Coefficient, other);
                        empty = cellfun(@isempty, this.Operators);
                        val(empty) = false;                        
                    end
                end
                return;
            end
                        
            % Otherwise, compare vs. another monomial
            assert(isa(other, 'Symbolic.Monomial'));
            
            % Never equal if scenarios do not match
            if this.Scenario ~= other.Scenario
                if this.IsScalar
                    val = false(size(other));
                else
                    val = false(size(this));
                end
                return
            end
            
            ops_equal = this.Hash == other.Hash;
            if this.IsScalar
                if other.IsScalar                
                    coefs_equal = this.Scenario.IsClose(...
                                    this.Coefficient, other.Coefficient);
                else
                    coefs_equal = this.Scenario.IsClose(...
                        repmat(this.Coefficient, size(other)), ...
                        other.Coefficient);
                end
            else
                if other.IsScalar                  
                    coefs_equal = this.Scenario.IsClose(this.Coefficient,...
                        repmat(other.Coefficient, size(this)));
                else                
                    coefs_equal = this.Scenario.IsClose(...
                        this.Coefficient, other.Coefficient);
                end
            end
            val = ops_equal & coefs_equal;
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
        % UPLUS Unary plus (typically does nothing, could degrade to zero).
        %
        % SYNTAX
        %   v = +mono
        %
        % RETURNS
        %   Either the same monomial object, or a monomial object with 
        %   numbers close to zero pruned.
        %
           [new_coefs, mask, mask_re, mask_im] = obj.Scenario.Prune(obj.Coefficient);
           if ~any(mask | mask_re | mask_im, 'all')
               val = obj;
           else
               if obj.IsScalar
                   if mask
                       new_ops = uint64.empty(1,0);
                   else
                       new_ops = obj.Operators;
                   end
               else
                   new_ops = Util.cell_mask(obj.Operators, mask, []);
               end
               
               val = Symbolic.Monomial(obj.Scenario, new_ops, new_coefs);
           end
        end
        
        function val = uminus(this)
        % UMINUS Unary minus; inverts (all!) coefficient signs
        %
        % SYNTAX
        %   v = -mono
        %
        % RETURNS
        %   A new Symbolic.Monomial, with the coefficient negated.
        %
            val = Symbolic.Monomial.InitForOverwrite(this.Scenario, size(this));
            val.Operators = this.Operators;
            val.Hash = this.Hash;
            val.Coefficient = -this.Coefficient;
            
           % Propagate symbol information
           val.symbol_id = this.symbol_id;
           val.symbol_conjugated = this.symbol_conjugated;
           val.re_basis_index = this.re_basis_index;
           val.im_basis_index = this.im_basis_index;
           
        end
        
        function val = times(lhs, rhs)
        % TIMES Element-wise multiplication .*
        
            % Pre-multiplication by a built-in type
            if ~isa(lhs, 'Symbolic.Monomial') 
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
            
            % Check dimensions
            this_size = size(this);
            other_size = size(other);            
            if ~isequal(this_size, other_size)
                if ~(this.IsScalar || numel(other) == 1)
                    error("Objects must be same size for _.*_, or one must be scalar.");
                end
            end
            
            % Handle case when other value is numeric
            if isnumeric(other)
                [new_coefs, prune_mask] = this.Scenario.Prune(this.Coefficient .* other);
                if this.IsScalar
                    if numel(other) == 1
                        if new_coefs ~= 0
                            % Scaled monomial
                            val = Symbolic.Monomial(this.Scenario, ...
                                this.Operators, new_coefs);                        
                        else
                            % Monomial zero
                            val = Symbolic.Monomial(this.Scenario, [], 0);                        
                        end
                    else
                        cell_size = num2cell(size(new_coefs));
                        new_ops = repelem({this.Operators}, cell_size{:});
                        new_ops = Util.cell_mask(new_ops, prune_mask, []);
                        val = Symbolic.Monomial(this.Scenario, ...
                            new_ops, new_coefs);
                        
                    end
                else
                    new_ops = Util.cell_mask(this.Operators, prune_mask, []);
                    val = Symbolic.Monomial(this.Scenario, new_ops, new_coefs);
                end
                return;
            end
            
            
            assert(this==lhs);
            assert(isa(other,'Symbolic.Monomial'));            
           
            % Handle various cases of Monomial with Monomial:
            [join_coefs, prune_mask] = ...
                lhs.Scenario.Prune(lhs.Coefficient .* rhs.Coefficient);
            
            if lhs.IsScalar && rhs.IsScalar                
                if join_coefs ~= 0
                    new_ops = [lhs.Operators, rhs.Operators];
                else
                    new_ops = [];
                end
            elseif lhs.IsScalar
                new_ops = cellfun(@(x) [lhs.Operators, x], ...
                    rhs.Operators, 'UniformOutput', false);                
                new_ops = Util.cell_mask(new_ops, prune_mask, []);                
            elseif rhs.IsScalar
                new_ops = cellfun(@(x) [x, rhs.Operators], ...
                    lhs.Operators, 'UniformOutput', false);                
                new_ops = Util.cell_mask(new_ops, prune_mask, []);                
            else
                new_ops = cellfun(@(x, y) [x, y], ...
                    lhs.Operators, rhs.Operators, 'UniformOutput', false);
                new_ops = Util.cell_mask(new_ops, prune_mask, []);
            end
            val = Symbolic.Monomial(this.Scenario, new_ops, join_coefs);            
        end
                
        function val = mtimes(lhs, rhs)
        % MTIMES Matrix multiplication.
        %
        % SYNTAX
        %   1. v = monomial * double
        %   2. v = double * monomial
        %   3. v = monomial * monomial
        %
        % RETURNS
        %   A new Symbolic.Monomial with appropriate coefficients and
        %   operators; except when double = 0, then 0.
        %
        % Due to class precedence, poly * mono and mono * poly cases are 
        % handled by Symbolic.Polynomial.mtimes.
        %
        % See also: ALGEBRAIC.POLYNOMIAL.MTIMES.
        %
            arguments
                lhs (:,:)
                rhs (:,:)
            end
            
            % Alias for .* if either side is a scalar.
            if numel(lhs)==1 || numel(rhs)==1
                val = times(lhs, rhs);
                return;
            end
            
            % TODO: Matrix multiplication.            
            error(['Matrix multiplication not yet defined for monomials. ',...
                   'For element-wise multiplication, use _.*_ instead.']);
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
        %   -double. Symbolic.Monomial, if monomial is proportional to 
        %   identity but not equal to double. Symbolic.Polynomial with two 
        %   terms otherwise.
        %
        % RETURNS (Suntax 3)
        %   Symbolic.Monomial if operator string in two monomials matches,
        %   and the coefficients are not negations of each other. 0 if the
        %   operator string of the two monomials matches, and the
        %   coefficients negate each other. Symbolic.Polynomial with two 
        %   terms otherwise.
        %   
        % Due to class precedence, "poly + mono" and "mono + poly" cases 
        % are handled by Symbolic.Polynomial.plus.
        %
        % See also: ALGEBRAIC.POLYNOMIAL.PLUS
        %
            % Add a scalar by a built-in type?
            if ~isa(lhs, 'Symbolic.Monomial')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
                        
            % Check dimensions
            this_size = size(this);
            other_size = size(other);            
            if ~isequal(this_size, other_size)
                if ~(this.IsScalar || numel(other) == 1)
                    error("Objects must be same size for _+_, or one must be scalar.");
                end
            end
                                      
            % Quick pass through for "+0" case
            if Util.is_scalar_zero(other)
                val = this;
                return;
            end
            
            % If other is numeric, convert to monomial:
            if isnumeric(other)
                other = Symbolic.Monomial.InitValue(this.Scenario, other);
            else
                assert(this == lhs);
                assert(isa(other, 'Symbolic.Monomial'));
                this.checkSameScenario(other);
            end
            
            % Handle remaining cases
            if this.IsScalar && other.IsScalar
                if isequal(this.Operators, other.Operators)
                    coef = this.Scenario.Prune(this.Coefficient ...
                                                + other.Coefficient);
                    if coef ~= 0
                        val = Symbolic.Monomial(this.Scenario, ...
                                                this.Operators, coef);
                    else
                        val = Symbolic.Monomial(this.Scenario, [], 0);
                    end
                else
                    val = Symbolic.Polynomial(this.Scenario, [this;other]);
                end
            elseif this.IsScalar % And RHS is not.
                if all(cellfun(@(x) isequal(this.Operators, x), ...
                        other.Operators))
                    [coef, mask] = this.Scenario.Prune(this.Coefficient...
                                                     + other.Coefficient);
                    if any(mask, 'all')
                       new_ops = other.Operators;
                       new_ops{mask} = [];
                       val = Symbolic.Monomial(this.Scenario, new_ops, coef); 
                    else
                       val = Symbolic.Monomial(this.Scenario, ...
                                               other.Operators, coef);
                    end
                else
                    mono_cells = other.split();
                    mono_cells = cellfun(@(x) [this; x], mono_cells, ...
                                       'UniformOutput', false);
                    val = Symbolic.Polynomial(this.Scenario, mono_cells);                    
                end
            elseif other.IsScalar % And LHS is not.
                if all(cellfun(@(x) isequal(x, this.Operators), ...
                        this.Operators))
                    [coef, mask] = this.Scenario.Prune(this.Coefficient...
                                                     + other.Coefficient);
                    if any(mask, 'all')
                        new_ops = this.Operators;
                        new_ops{mask} = [];
                        val = Symbolic.Monomial(this.Scenario, new_ops, coef);
                    else
                        val = Symbolic.Monomial(this.Scenario, ...
                            this.Operators, coef);
                    end
                else 
                    mono_cells = this.split();
                    mono_cells = cellfun(@(x) [x; other], mono_cells, ...
                                       'UniformOutput', false);
                    val = Symbolic.Polynomial(this.Scenario, mono_cells);
                end
            else % Nothing is scalar
                if all(cellfun(@(x, y) isequal(x, y), ...
                        this.Operators, other.Operators))
                    [coef, mask] = this.Scenario.Prune(this.Coefficient ...
                                                      + other.Coefficient);
                    if any(mask, 'all')
                        new_ops = this.Operators;
                        new_ops{mask} = [];
                        val = Symbolic.Monomial(this.Scenario, new_ops, coef);
                    else
                        val = Symbolic.Monomial(this.Scenario, ...
                            this.Operators, coef);
                    end
                else                    
                    mono_this = this.split();
                    mono_other = other.split();
                    mono_cells = cellfun(@(x, y) [x; y], ...
                                        mono_this, mono_other, ...
                                        'UniformOutput', false);
                    val = Symbolic.Polynomial(this.Scenario, mono_cells);
                end
            end
        end
        
        
        function val = minus(lhs, rhs)
        % MINUS Subtraction, defined as addition of lhs with -rhs.
        %
        % See also: ALGEBRAIC.MONOMIAL.PLUS, ALGEBRAIC.MONOMIAL.UMINUS
        %
            val = lhs + -rhs;
        end
        
        function val = conj(obj)
        % CONJ Conjugation (without transpose).
            [conj_ops, negated, hashes] = obj.Scenario.Conjugate(obj.Operators);
            neg_mask = ones(size(negated));
            neg_mask(negated) = -1;
            new_coefs = conj(obj.Coefficient) .* neg_mask;
            
            val = Symbolic.Monomial.InitForOverwrite(obj.Scenario, size(obj));
            val.Operators = conj_ops;
            val.Coefficient = new_coefs;
            val.Hash = hashes;
            
        end
        
        function val = ctranspose(obj)
        % CTRANSPOSE Complex conjugation / transpose.
        
            % Do conjugation of operators and coefficients
            [conj_ops, negated, hashes] = obj.Scenario.Conjugate(obj.Operators);
            neg_mask = ones(size(negated));
            neg_mask(negated) = -1;
            new_coefs = conj(obj.Coefficient) .* neg_mask;
                   
            % Transpose elements
            switch obj.DimensionType
                case 0 %SCALAR                                
                case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
                    conj_ops = conj_ops.';                   
                    new_coefs = new_coefs.';
                    hashes= hashes.';                    
                otherwise
                    permutation = [2, 1, 3:numel(obj.Dimensions)];
                    conj_ops = permute(conj_ops, permutation);
                    new_coefs = permute(new_coefs, permutation);
                    hashes = permute(hashes, permutation);                    
            end

            % Make new object
            val = Symbolic.Monomial.InitForOverwrite(obj.Scenario, size(conj_ops));
            val.Operators = conj_ops;
            val.Coefficient = new_coefs;
            val.Hash = hashes;
        end
        
        
        function val = transpose(obj)
        % CTRANSPOSE Transpose (without complex conjugation).
        
            % Transpose elements
            switch obj.DimensionType
                case 0 %SCALAR      
                    val = obj;
                    return
                case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
                    trans_ops = obj.Operators.';
                    new_coefs = obj.Coefficient.';
                    hashes = obj.Hash.';
                otherwise
                    permutation = [2, 1, 3:numel(obj.Dimensions)];
                    trans_ops = permute(obj.Operators, permutation);
                    new_coefs = permute(obj.Coefficient, permutation);
                    hashes = permute(obj.Hash, permutation);                    
            end

            % Make new object
            val = Symbolic.Monomial.InitForOverwrite(obj.Scenario,...
                                                     size(trans_ops));
            val.Operators = trans_ops;
            val.Coefficient = new_coefs;
            val.Hash = hashes;
        end
    end
            
    %% Apply substitution rules
    methods        
        function val = ApplyRules(obj, rulebook )
        % APPLYRULES Transform moments of matrix according to rulebook.
        %
        % Effectively applies rules to each constituent matrix in turn.
        % 
            arguments
                obj (1,1) Symbolic.Monomial
                rulebook (1,1) MomentRulebook
            end
       
            % Promote to polynomial, then apply
            obj_as_poly = Symbolic.Polynomial(obj.Scenario, obj);
            val = obj_as_poly.ApplyRules(rulebook);
        end 
    end
    
    %% Virtual methods
    methods(Access=protected)
        function [re, im] = calculateCoefficients(obj)
            
            % Early exit if we can't get symbol information...
            if ~all(obj.SymbolId >= 0, 'all')
                error("Symbols are not yet all registered in MatrixSystem.");
            end 
            
            
            % Real coefficients
            switch obj.DimensionType
                case 0 % SCALAR                    
                    sys = obj.Scenario.System;
                    re = complex(sparse(double(sys.RealVarCount), 1));
                    if obj.re_basis_index > 0
                        re(obj.re_basis_index) = obj.Coefficient;
                    end
                case 1
                    re = obj.makeColOfRealCoefficientsRV();
                case 2 % ROW- and COL-VECTOR
                    re = obj.makeColOfRealCoefficients(1);
                otherwise                    
                    dims = size(obj);
                    if length(dims) <= 2
                        re = cell(dims(2), 1);
                    else
                        celldims = cell(dims);
                        re = cell(celldims{2:end});
                    end

                    % Iterate over remaining dimensions
                    for idx = 1:prod(dims(2:end))
                        re{idx} = obj.makeColOfRealCoefficients(idx);
                    end
            end
            
            % Imaginary coefficients
            switch obj.DimensionType
                case 0 % SCALAR
                    sys = obj.Scenario.System;
                    im = complex(sparse(double(sys.ImaginaryVarCount), 1));
                    if obj.im_basis_index > 0
                        if obj.SymbolConjugated
                            im(obj.im_basis_index) = -1i * obj.Coefficient;
                        else
                            im(obj.im_basis_index) = 1i * obj.Coefficient;
                        end
                    end
                case 1
                	im = obj.makeColOfImaginaryCoefficientsRV();
                case 2 % ROW- and COL-VECTOR
                    im = obj.makeColOfImaginaryCoefficients(1);
                otherwise
                     dims = size(obj);
                    if length(dims) <= 2
                        im = cell(dims(2), 1);
                    else
                        celldims = cell(dims);
                        im = cell(celldims{2:end});
                    end

                    % Iterate over remaining dimensions
                    for idx = 1:prod(dims(2:end))
                        im{idx} = obj.makeColOfImaginaryCoefficients(idx);
                    end
            end
        end
                
        function spliceOut(output, source, indices)
            spliceOut@Symbolic.ComplexObject(output, source, indices);

            if source.IsScalar
                assert(output.IsScalar);
                output.Operators = source.Operators;
            else
                output.Operators = source.Operators{indices{:}};
            end
            output.Coefficient = source.Coefficient(indices{:});
            output.Hash = source.Hash(indices{:});
            
            output.symbol_id = source.symbol_id(indices{:});
            output.symbol_conjugated = source.symbol_conjugated(indices{:});
            output.re_basis_index = source.re_basis_index(indices{:});
            output.im_basis_index = source.im_basis_index(indices{:});
            
        end
        
        function [output, matched] = spliceProperty(obj, indices, propertyName)
            switch propertyName
                case 'Operators'
                    if obj.IsScalar
                        output = obj.Operators;
                    else
                        output = obj.Operators(indices{:});
                    end
                    matched = true;
                case 'Coefficient'
                    output = obj.Coefficient(indices{:});
                    matched = true;
                case 'Hash'
                    output = obj.Hash(indices{:});
                    matched = true;
                case 'FoundSymbol'
                    output = obj.FoundSymbol(indices{:});
                    matched = true;
                case 'SymbolId'
                    output = obj.SymbolId(indices{:});
                    matched = true;
                case 'SymbolConjugated'
                    output = obj.SymbolConjugated(indices{:});
                    matched = true;
                case 'RealBasisIndex'
                    output = obj.RealBasisIndex(indices{:});
                    matched = true;
                case 'ImaginaryBasisIndex'        
                    output = obj.ImaginaryBasisIndex(indices{:});
                    matched = true;
                otherwise
                    [output, matched] = ...
                        spliceProperty@Symbolic.ComplexObject(obj, ...
                                                              indices, ...
                                                              propertyName);
            end
        end
        
        function mergeIn(obj, merge_dim, offsets, objects)
            merge_type = mergeIn@Symbolic.ComplexObject(obj, merge_dim, ...
                                                        offsets, objects);

            % If scalar, promote operator list to cell before merge.
            if merge_type == 0 || merge_type == 1
                m_operators = (cellfun(@(x) {x.Operators}, ...
                              objects, 'UniformOutput', false));
            else                                                   
                m_operators = (cellfun(@(x) x.Operators, ...
                              objects, 'UniformOutput', false));               
            end
            obj.Operators = cat(merge_dim, m_operators{:});
            
            m_coefficient = (cellfun(@(x) x.Coefficient, ...
                          objects, 'UniformOutput', false));
            obj.Coefficient = cat(merge_dim, m_coefficient{:});
            
            m_hash = (cellfun(@(x) x.Hash, ...
                          objects, 'UniformOutput', false));
            obj.Hash = cat(merge_dim, m_hash{:});
            
            m_symbol_id = (cellfun(@(x) x.symbol_id, ...
                          objects, 'UniformOutput', false));
            obj.symbol_id = cat(merge_dim, m_symbol_id{:});
              
            m_symbol_conjugated = (cellfun(@(x) x.symbol_conjugated, ...
                                  objects, 'UniformOutput', false));
            obj.symbol_conjugated = cat(merge_dim, m_symbol_conjugated{:});
                      
            m_re_basis_index = (cellfun(@(x) x.re_basis_index, ...
                                  objects, 'UniformOutput', false));
            obj.re_basis_index = cat(merge_dim, m_re_basis_index{:});

            m_im_basis_index = (cellfun(@(x) x.im_basis_index, ...
                                  objects, 'UniformOutput', false));
            obj.im_basis_index = cat(merge_dim, m_im_basis_index{:});            
        end        
        
        function str = makeObjectName(obj)
            str = strings(size(obj));
            if obj.IsScalar
                str(1) = ...
                    obj.makeOneName(obj.Operators, obj.Coefficient);
            else            
                for idx = 1:numel(obj)
                    str(idx) = obj.makeOneName(obj.Operators{idx}, ...
                                        obj.Coefficient(idx));
                end
            end
        end
        
    end
    
    %% Internal methods
    methods (Access=private)
        function loadSymbolInfoOrError(obj)
            % Cached value?
            if any(obj.symbol_id == -1)
                obj.loadSymbolInfo();
                
                if any(obj.symbol_id == -1)
                    error("Some symbols were not found in matrix system.");
                end
            end
        end
        
        function val = makeColOfRealCoefficients(obj, index)
            re_var_count = double(obj.Scenario.System.RealVarCount);
            num_cols = size(obj, 1);
            
            rebi = double(reshape(obj.re_basis_index(:, index), 1, []));
            rebi_mask = rebi > 0;

            val = complex(sparse(re_var_count, num_cols));
            idx = re_var_count * (0:(num_cols-1)) + rebi;
            idx = idx(rebi_mask);

            val(idx) = obj.Coefficient(rebi_mask, index);
        end
        
        function val = makeColOfImaginaryCoefficients(obj, index)
            im_var_count = double(obj.Scenario.System.ImaginaryVarCount);
            num_cols = size(obj, 1);
            
            imbi = double(reshape(obj.im_basis_index(:, index), 1, []));
            imbi_mask = imbi > 0;
            val = complex(sparse(im_var_count, num_cols));
            idx = im_var_count * (0:(num_cols-1)) + imbi;
            idx = idx(imbi_mask);
            
            values = (1i * ~obj.symbol_conjugated(imbi_mask)) + ...
                -1i * obj.symbol_conjugated(imbi_mask);
            values = values .* obj.Coefficient(imbi_mask, index);
            val(idx) = values;            
        end
                
        function val = makeColOfRealCoefficientsRV(obj)
            re_var_count = double(obj.Scenario.System.RealVarCount);
            num_cols = size(obj, 2);
            
            rebi = double(obj.re_basis_index);
            rebi_mask = rebi > 0;

            val = complex(sparse(re_var_count, num_cols));
            idx = re_var_count * (0:(num_cols-1)) + rebi;
            idx = idx(rebi_mask);

            val(idx) = obj.Coefficient(rebi_mask);
        end
        
        function val = makeColOfImaginaryCoefficientsRV(obj)
            im_var_count = double(obj.Scenario.System.ImaginaryVarCount);
            num_cols = size(obj, 2);
            
            imbi = double(obj.im_basis_index);
            imbi_mask = imbi > 0;
            val = complex(sparse(im_var_count, num_cols));
            idx = im_var_count * (0:(num_cols-1)) + imbi;
            idx = idx(imbi_mask);
            
            values = (1i * ~obj.symbol_conjugated(imbi_mask)) + ...
                -1i * obj.symbol_conjugated(imbi_mask);
            values = values .* obj.Coefficient(imbi_mask);
            val(idx) = values;
            
        end
               
        function success = loadSymbolInfo(obj)

            % Set to default (and flag failure) if no matrix system yet.
            if ~obj.Scenario.HasMatrixSystem
                 obj.setDefaultSymbolInfo();
                 success = false;
                 return;
            end
            
            sys = obj.Scenario.System;
                        
            if obj.IsScalar
                row = mtk('symbol_table', sys.RefId, obj.Operators);
                 if (isa(row, 'logical') && (row == false))
                    obj.setDefaultSymbolInfo();
                    success = false;
                else
                    obj.symbol_id = int64(row.symbol);
                    obj.re_basis_index = uint64(row.basis_re);
                    obj.im_basis_index = uint64(row.basis_im);
                    obj.symbol_conjugated = logical(row.conjugated);
                    success = true;
                 end
            else
                rows = mtk('symbol_table', sys.RefId, obj.Operators);
                obj.setDefaultSymbolInfo(); % preallocate for speed
                for idx=1:numel(rows)
                    row = rows(idx);
                    if (row.symbol >= 0)
                        obj.symbol_id(idx) = row.symbol;
                        obj.re_basis_index(idx) = uint64(row.basis_re);
                        obj.im_basis_index(idx) = uint64(row.basis_im);
                        obj.symbol_conjugated(idx) = logical(row.conjugated);                        
                    end
                end
                success = all(obj.symbol_id >= 0);
            end
        end
        
        function setDefaultSymbolInfo(obj)
            if obj.IsScalar()
                obj.symbol_id = -1;
                obj.symbol_conjugated = false;
                obj.re_basis_index = 0;
                obj.im_basis_index = 0;
            else
                
                dims = size(obj);
                obj.symbol_id = int64(ones(dims)) * -1;
                obj.symbol_conjugated = false(dims);               
                obj.re_basis_index = uint64(zeros(dims));
                obj.im_basis_index = uint64(zeros(dims));
            end            
        end
        
        function val = makeOneName(obj, opers, coef)            
            if ~isempty(opers)
                % FIXME: Proper name-context object
                op_names = obj.Scenario.Rulebook.ToStringArray(opers);
                
                val = "<" + join(op_names, '') + ">";
                
                if coef ~= 1.0
                    if coef == -1.0
                        val = "-" + val;
                    else
                        if ~isreal(coef)
                            if real(coef) == 0
                                if imag(coef) == 18
                                    val = "i" + val;
                                elseif imag(coef) == -1
                                    val = "-i" + val;
                                else
                                    val = sprintf("%gi %s", imag(coef), ...
                                                  val);
                                end
                            else
                                val = sprintf("(%g+%gi) %s", ...
                                              real(coef), imag(coef), val);
                            end
                        else
                            val = sprintf("%g %s", coef, val);
                        end
                    end
                end            
            else
                val = sprintf("%g", coef);
            end
        end            
    end
end
