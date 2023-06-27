classdef (InferiorClasses={?Symbolic.Monomial}) ...
        Polynomial < Symbolic.ComplexObject
%POLYNOMIAL A polynomial expression of operators (or their moments).
    properties
        Constituents = Symbolic.Monomial.empty(1,0)
    end
        
    properties(Dependent)
        IsZero
        OperatorCell
        SymbolCell        
    end
    
    properties(Access=private)
        symbol_cell = cell(0, 0);        
        operator_cell = cell(0, 0);
        done_sc = false;
        done_oc = false;
    end
    
    properties(Constant,Access=private)
        err_bad_ctr_input = ['Second argument must be either a Monomial',...
                              ' array, or a cell array of Monomials'];
                          
        err_missing_symbol = ['Not all constituent symbols were ', ...
                              'identified in the MatrixSystem.'];
    end

    methods
        function obj = Polynomial(setting, varargin)
        % POLYNOMIAL Constructs an algebraic polynomial object
        %
        % Syntax:
        %  1.   p = Polynomial(setting, [vector of monomials]
        %  2.   p = Polynomial(setting, {Cell of [vector of monomials]}
        %  3.   p = Polynomial(setting, 'overwrite', [creation size])
        %
        % Syntax 1 creates a scalar polynomial.
        % Syntax 2 creates an array of polynomials.
        % Syntax 3 creates an uninitialized (scalar/array of) polynomial(s).
        %
        
            % Check argument 1
            if nargin < 1
                error("Scenario must be provided.");
            elseif ~isa(setting, 'Abstract.Scenario')
                error("First argument must be a scenario.");
            end

            % Check argument 2
            if nargin < 2
                create_dimensions = [1, 1];
                array_construction = false;
                zero_construction = true;
                constituents = Symbolic.Monomial.empty(1,0);
            else
                if isa(varargin{1}, 'Symbolic.Monomial')
                    create_dimensions = [1, 1];
                    array_construction = false;
                    constituents = varargin{1};
                    zero_construction = isempty(constituents);
                elseif isa(varargin{1}, 'cell')
                    constituents = varargin{1};
                    create_dimensions = size(constituents);
                    if ~all(cellfun(@(x) isa(x, 'Symbolic.Monomial'), ...
                            constituents))
                        error(Symbolic.Polynomial.err_bad_ctr_input);
                    end
                    if numel(constituents) == 1
                        constituents = constituents{1};
                        create_dimensions = [1, 1];
                        array_construction = false;                        
                    else
                        array_construction = true;
                    end
                    zero_construction = false;
                elseif (isstring(varargin{1}) || ischar(varargin{1})) ...
                        && strcmp(varargin{1}, 'overwrite')
                    create_dimensions = double(varargin{2});
                    array_construction = prod(create_dimensions) ~= 1;
                    zero_construction = true;                    
                else
                    error(Symbolic.Polynomial.err_bad_ctr_input);
                end
            end

            % Parent c'tor
            obj = obj@Symbolic.ComplexObject(setting, create_dimensions);
            
            % Quick construction for empty object
            if zero_construction
                if array_construction
                    cell_dims = num2cell(create_dimensions);
                    obj.Constituents = ...
                        repelem({Symbolic.Monomial.empty(0,1)}, cell_dims{:});                     
                else
                    obj.Constituents = Symbolic.Monomial.empty(0,1);
                end
                return;
            end
            
            if ~array_construction
                obj.Constituents = obj.orderAndMerge(constituents);
            else
                obj.Constituents = cell(size(constituents));
                for idx = 1:numel(constituents)
                    obj.Constituents{idx} = obj.orderAndMerge(constituents{idx});
                end                
            end
        end    
    end
    
    
    %% Named constructor
    methods(Static)
        function obj = InitForOverwrite(setting, dimensions)
        % INITFOROVERWRITE Create a blank polynomial object
        %
        % See also: SYMBOLIC.POLYNOMIAL.POLYNOMIAL
        %
            obj = Symbolic.Polynomial(setting, 'overwrite', dimensions);
        end
    end
    
    %% Convertors
    methods
        function mono = Symbolic.Monomial(obj)
            error("Down-conversion not yet implemented.");
        end
    end
    
    %% Dependent variables
    methods
        function val = get.IsZero(obj)
            if obj.IsScalar
                val = isempty(obj.Constituents);
            else
                val = cellfun(@(x) isempty(x), obj.Constituents);
            end
        end
        
        function val = get.OperatorCell(obj)
            if ~obj.done_oc
                obj.makeOperatorCell();
                if ~obj.done_oc
                    error("Could not make operator cell.");
                end
            end
            val = obj.operator_cell;
        end
        
        function val = get.SymbolCell(obj)
            if ~obj.done_sc
                obj.makeSymbolCell();
                if ~obj.done_sc
                    error("Could not make symbol cell.");
                end
            end
            val = obj.symbol_cell;
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
                obj (1,1) Symbolic.Polynomial
                level (1,1) uint64
            end
            
            if ~obj.IsScalar
                error("Can only generate localizing matrices for scalar polynomials.");
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
        % Symbolic.Monomial.EQ), or for the polynomial to be zero as a
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
       
            if isa(lhs, 'Symbolic.Polynomial')
                this = lhs;
                other = rhs;
            else
                this = rhs;
                other = lhs;
            end
            
            % Handle comparison with numeric
            if isnumeric(other)
                % Make array of scalar values, taking NaN if not scalar.
                this_values = NaN(size(this));
                if this.IsScalar
                    if numel(this.Constituents) == 0
                        this_values = 0;
                    elseif numel(this.Constituents) == 1 && ...
                            isempty(this.Constituents(1).Operators)
                        this_values = this.Constituents.Coefficient;
                    end
                else                    
                    for idx=1:numel(this.Constituents)
                        if numel(this.Constituents{idx}) == 0
                            this_values(idx)= 0;
                        elseif numel(this.Constituents{idx}) == 1 && ...
                            isempty(this.Constituents{idx}.Operators)
                            this_values(idx) = this.Constituents{idx}.Coefficient;
                        end
                    end
                end
                % Compare vs. other
                val = this_values == other;
                return;            
            end
            
            % Never equal if scenarios do not match
            assert(isa(other, 'Symbolic.ComplexObject'));
            if this.Scenario ~= other.Scenario
                if this.IsScalar
                    val = false(size(other));
                else
                    val = false(size(this));
                end
                return
            end
                    
            % Handle comparison with monomials
            if isa(other, 'Symbolic.Monomial')
                if this.IsScalar
                    if other.IsScalar
                        val = (numel(this.Constituents) == 1  ...
                            && this.Constituents == other) ...
                            || (numel(this.Constituents) == 0 ...
                            && other.IsZero);
                    else
                        if this.IsZero
                            val = other.IsZero;
                        elseif numel(this.Constituents) == 1
                            val = this.Constituents(1) == other;
                        else
                            val = false(size(other));
                        end
                    end
                else
                    if other.IsScalar
                        if other.IsZero
                            val = this.IsZero;
                        else
                            val = cellfun(@(x) (numel(x)==1 && x == other),...
                                this.Constituents);
                        end
                    else
                        % First of all, match places where this and other are zero.
                        val = this.IsZero & other.IsZero;
                        where_one = cellfun(@(x) (numel(x)==1),...
                            this.Constituents);
                        where_one = where_one & ~other.IsZero;
                        
                        for idx=1:numel(this)
                            if where_one(idx)
                                val(idx) = ...
                                    isequal(this.Constituents{idx}.Hash, other.Hash(idx)) ...
                                  && this.Scenario.IsClose(this.Constituents{idx}.Coefficient, ...
                                                          other.Coefficient(idx));
                                       
                            end
                        end
                    end
                end
                return;
            end
            
            assert (isa(other, 'Symbolic.Polynomial'))
            errror("TODO: Poly == poly");
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
        function val = times(lhs, rhs)
       % MTIMES Elementwise Multiplication.
       %
       % SYNTAX
       %   1. v = polynomial .* double
       %   2. v = double .* polynomial 
       %   3. v = polynomial .* monomial
       %   4. v = monomial .* polynomial 
       %   5. v = polynomial_A .* polynomial_B
       %
       % RETURNS
       %   A new Symbolic.Polynomial with appropriate coefficients and
       %   operators.
       %
       % See also: MONOMIAL.TIMES
       %
            % Pre-multiplication by a built-in type, or Symbolic.Monomial
            if ~isa(lhs, 'Symbolic.Polynomial')
                this = rhs;
                other = lhs;
                premult = true;
            else
                this = lhs;
                other = rhs;
                premult = false;
            end
                    
            % Check dimensions
            this_size = size(this);
            other_size = size(other);            
            if ~isequal(this_size, other_size)
                if ~(this.IsScalar || numel(other) == 1)
                    error("Objects must be same size for _+_, or one must be scalar.");
                end
            end
            
            % Handle numerics (commutes!)
            if isnumeric(other)                
                if this.IsScalar 
                    val = Symbolic.Polynomial(this.Scenario, other * this.Constituents);
                else                    
                    if numel(other)==1
                        monos = cellfun(@(x) (other * x), this.Constituents, 'UniformOutput', false);
                        val = Symbolic.Polynomial(this.Scenario, monos);
                    else
                        val = Symbolic.Polynomial(this.Scenario, ...
                              cellfun(@(x,y) (x * y), this.Constituents,...
                                                      num2cell(other),  'UniformOutput', false));
                    end
                end
                return;
            end
            
            % Handle monomials (including broadcasting)
            if isa(other, 'Symbolic.Monomial')
                if premult
                    if this.IsScalar
                        if other.IsScalar
                            new_monos = other .* this.Constituents;
                        else
                            new_monos = cell(size(other));
                            for idx=1:numel(new_monos)
                                new_monos{idx} = other(idx) .* this.Constituents;
                            end     
                        end
                    else
                        if other.IsScalar
                            new_monos = cellfun(@(x) (other .* x),...
                                this.Constituents, 'UniformOutput', false);
                        else
                            new_monos = cell(size(other));
                            for idx=1:numel(new_monos)
                                new_monos{idx} = other(idx) .* this.Constituents{idx};
                            end                            
                        end
                    end
                else
                   if this.IsScalar
                        if other.IsScalar
                            new_monos = this.Constituents .* other;
                        else
                            new_monos = cell(size(other));
                            for idx=1:numel(new_monos)
                                new_monos{idx} = this.Constituents .* other(idx);
                            end                            
                        end
                    else
                        if other.IsScalar
                            new_monos = cellfun(@(x) (x .* other),...
                                this.Constituents, 'UniformOutput', false);
                        else
                            new_monos = cell(size(other));
                            for idx=1:numel(new_monos)
                                new_monos{idx} = this.Constituents{idx} .* other(idx);
                            end                            
                        end
                    end
                end
                val = Symbolic.Polynomial(this.Scenario, new_monos);
                return;
            end
            
            % Handle polynomials
            assert(isa(rhs, 'Symbolic.Polynomial'))
            if lhs.IsScalar 
                if rhs.IsScalar
                    monomials = ...
                        lhs.multiplyMonomialVectors(lhs.Constituents, ...
                                                    rhs.Constituents);
                else
                    monomials = cell(size(rhs));
                    for idx=1:numel(rhs)
                        monomials{idx} = ...
                            lhs.multiplyMonomialVectors(lhs.Constituents, ...
                                                        rhs.Constituents{idx});
                    end
                end
            elseif rhs.IsScalar
                monomials = cell(size(lhs));
                for idx=1:numel(lhs)
                    monomials{idx} = ...
                        lhs.multiplyMonomialVectors(lhs.Constituents{idx}, ...
                                                    rhs.Constituents);
                end
            else
                assert(isequal(size(lhs), size(rhs)));
                monomials = ...
                    cellfun(@(x,y) (lhs.multiplyMonomialVectors(x,y)), ...
                            lhs.Constituents, rhs.Constituents, ...
                            'UniformOutput', false);
            end
            val = Symbolic.Polynomial(lhs.Scenario, monomials);
        end
        
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
       %   A new Symbolic.Polynomial with appropriate coefficients and
       %   operators.
       %
       % See also: Monomial.mtimes
       %
              
            % Alias for .* if either side is a scalar.
            if numel(lhs)==1 || numel(rhs)==1
                val = times(lhs, rhs);
                return;
            end
            
            % TODO: Matrix multiplication.            
            error(['Matrix multiplication not yet defined for polynomials. ',...
                   'For element-wise multiplication, use _.*_ instead.']);
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
        %   Either 0, an Symbolic.Monomial, or Symbolic.Polynomial.
        %   Numeric 0 is returned if all terms cancel out after addition.
        %   Symbolic.Monomial is returned if all but one term cancels out.
        %   Otherwise, Symbolic.Polynomial is returned.
        %
        % See also: ALGEBRAIC.MONOMIAL.PLUS
        %
            
            % Which are we??
            if ~isa(lhs, 'Symbolic.Polynomial')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            % Is other side built-in numeric; if so, cast to monomial
            if isnumeric(other)
                other = Symbolic.Monomial.InitValue(this.Scenario, other);
            end
                       
            % Check objects are from same scenario
            this.checkSameScenario(other);
            
            % Handle Monomial append case
            if isa(other, 'Symbolic.Monomial')
                if this.IsScalar
                    if other.IsScalar
                        val = Symbolic.Polynomial(this.Scenario, ...
                                        [this.Constituents; other]);
                    else
                        other = other.split();                        
                        val = Symbolic.Polynomial(this.Scenario, ...
                            cellfun(@(x) [this.Constituents; x], other,...
                                    'UniformOutput', false));
                    end
                else
                    if other.IsScalar
                       val = Symbolic.Polynomial(this.Scenario, ...
                            cellfun(@(x) [x; other], ...
                                this.Constituents, 'UniformOutput', false));
                    else
                        other = other.split();
                        val = Symbolic.Polynomial(this.Scenario, ...
                            cellfun(@(x, y) [x; y], ...
                                this.Constituents, other, ...
                                'UniformOutput', false));
                    end                    
                end                
                return
            end
            
            % Handle Polynomial append case
            assert(isa(other, 'Symbolic.Polynomial'));
            
            if this.IsScalar
                if other.IsScalar
                    val = Symbolic.Polynomial(this.Scenario, ...
                                    [this.Constituents; ...
                                     other.Constituents]);
                else
                    val = Symbolic.Polynomial(this.Scenario, ...
                        cellfun(@(x) [this.Constituents; x], ...
                                other.Constituents, ...
                                'UniformOutput', false));
                end
            else
                if other.IsScalar
                   val = Symbolic.Polynomial(this.Scenario, ...
                        cellfun(@(x) [x; other.Constituents], ...
                            this.Constituents, 'UniformOutput', false));
                else
                    val = Symbolic.Polynomial(this.Scenario, ...
                        cellfun(@(x, y) [x; y], ...
                            this.Constituents, other.Constituents, ...
                            'UniformOutput', false));
                end
            end            
        end
        
        function val = uplus(obj)
        % UPLUS Unary plus.
        % If single-element polynomial, will degrade to monomial.
        % If equal to zero, will degrade to numeric 0.
            val = obj;    
            return 
        end
        
        function val = uminus(obj)
        % UMINUS Unary minus.
        % Creates new polynomial, with all coefficients negated.
        %
            if obj.IsScalar
                val = Symbolic.Polynomial(obj.Scenario, -obj.Constituents);
            else
                val = Symbolic.Polynomial(obj.Scenario, ...
                         cellfun(@(x) unminus(x), obj.Constituents,...
                                 'UniformOutput', false));
            end            
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
 
        
         function val = conj(obj)
            % CONJ Conjugation (without transpose).
            if obj.IsScalar
                val = Symbolic.Polynomial(obj.Scenario, ...
                                          conj(obj.Constituents));
            else
                new_constituents = cellfun(@(x) conj(x), ...
                                           obj.Constituents, ...
                                           'UniformOutput', false);
                val = Symbolic.Polynomial(obj.Scenario, new_constituents);
            end
         end
        
        
        function val = ctranspose(obj)
        % CTRANSPOSE Complex conjugation / transpose.
               
            % Conjugate each polynomial's constituents
            if obj.IsScalar
                conj_const = conj(obj.Constituents);
            else
                conj_const = cellfun(@(x) conj(x), obj.Constituents, ...
                                     'UniformOutput', false);
            end
        
            % Transpose elements
            switch obj.DimensionType
                case 0 %SCALAR                                
                case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
                    conj_const = conj_const.';
                otherwise
                    permutation = [2, 1, 3:numel(obj.Dimensions)];
                    conj_const = permute(conj_const, permutation);
            end

            % Make new object
            val = Symbolic.Polynomial(obj.Scenario, conj_const);            
        end
        
        
        function val = transpose(obj)
        % CTRANSPOSE Transpose (without complex conjugation).
              % Transpose elements
            switch obj.DimensionType
                case 0 %SCALAR    
                    val = obj;
                    return;
                case {1, 2, 3} % ROW-VEC, COL-VEC, MATRIX
                    trans_const = obj.Constituents.';
                otherwise
                    permutation = [2, 1, 3:numel(obj.Dimensions)];
                    trans_const = permute(obj.Constituents, permutation);
            end

            % Make new object
            val = Symbolic.Polynomial(obj.Scenario, trans_const);            
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
                obj (1,1) Symbolic.Polynomial
                rulebook (1,1) MomentRulebook
            end
            
            % Scenarios must match
            if (rulebook.Scenario ~= obj.Scenario)
                error(obj.err_mismatched_scenario);
            end
            
            % Get transformed version of polynomial
            as_symbol_cell = obj.SymbolCell();
            output_sequences = mtk('apply_moment_rules', ...
                obj.Scenario.System.RefId, rulebook.RulebookId, ...
                'output', 'sequences', as_symbol_cell);
            
            % Construct new, transformed polynomial
            val = Symbolic.Polynomial(obj.Scenario, output_sequences);
            
            % Degrade to monomial if only single element.
            if length(val.Constituents) == 1
                val = val.Constituents(1);
            end
        end
    end   
    
    
    %% Virtual methods
    methods(Access=protected)
        function [re, im] = calculateCoefficients(obj)
            % Early exit if we can't get symbol information for all parts...
            if obj.IsScalar                
                if ~all([obj.Constituents.FoundSymbol], 'all')
                    error(obj.err_missing_symbol);
                end
            else
                if ~all(cellfun(@(c) all(c.FoundSymbol, 'all'), ...
                        obj.Constituents), 'all')
                    error(obj.err_missing_symbol);
                end
            end
            
            
            switch obj.DimensionType
                case 0 % SCALAR
                    re = obj.Scenario.Prune(...
                            sum(obj.Constituents.RealCoefficients, 2));
                    im = obj.Scenario.Prune(...
                            sum(obj.Constituents.ImaginaryCoefficients, 2));
                    
                case {1, 2} % ROW-VECTOR, COL-VECTOR                    
                    cell_re = cellfun(@(x) obj.Scenario.Prune(...
                                           sum(x.RealCoefficients, 2)), ...
                                      obj.Constituents, ...
                                  'UniformOutput', false);
                    re = horzcat(cell_re{:});
                    cell_im = cellfun(@(x) obj.Scenario.Prune(...
                                           sum(x.ImaginaryCoefficients, 2)), ...
                                      obj.Constituents, ...
                                  'UniformOutput', false);
                    im = horzcat(cell_im{:});
                
                otherwise                    
                    cell_re = cellfun(@(x) obj.Scenario.Prune(...
                                           sum(x.RealCoefficients, 2)), ...
                                      obj.Constituents, ...
                                  'UniformOutput', false);
                              
                    cell_im = cellfun(@(x) obj.Scenario.Prune(...
                                           sum(x.ImaginaryCoefficients, 2)), ...
                                      obj.Constituents, ...
                                  'UniformOutput', false);
                              
                    % Contract index 1 into matrix, distribute over cell
                    short_dims = size(cell_re);
                    short_dims = short_dims(2:end);
                    if (numel(short_dims) == 1)
                        re = cell(short_dims, 1);
                        im = cell(short_dims, 1);
                        for idx=1:prod(short_dims)
                            re{idx} = cell_re{:, idx};
                            im{idx} = cell_im{:, idx};
                        end    
                    else
                        re = cell(short_dims);
                        im = cell(short_dims);
                        for idx=1:prod(short_dims)
                            re_cell_col = cell_re{:, idx};
                            re{idx} = horzcat(re_cell_col{:});
                            im_cell_col = cell_im{:, idx};
                            im{idx} = horzcat(im_cell_col{:});
                        end
                    end
                    
            end  
        end
        
        
        function spliceOut(output, source, indices)
            spliceOut@Symbolic.ComplexObject(output, source, indices);

            if source.IsScalar
                assert(output.IsScalar);
                output.Constituents = source.Constituents;
            else
                output.Constituents = source.Constituents(indices{:});
            end
        end
        
        function [output, matched] = spliceProperty(obj, indices, propertyName)
            switch propertyName
                case 'Constituents'
                    if obj.IsScalar
                        output = obj.Constituents;
                    else
                        output = obj.Constituents(indices{:});
                    end
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

            % If scalar, promote constituent list to cell before merge.
            switch merge_type
                case {0, 1} % Scalar to row/col vec                               
                    m_constituents = (cellfun(@(x) {x.Constituents}, ...
                                      objects, 'UniformOutput', false));
                % FIXME: Row vector to larger-row-vec
                otherwise
                    m_constituents = (cellfun(@(x) x.Constituents, ...
                                   objects, 'UniformOutput', false));               
            end

            obj.Constituents = cat(merge_dim, m_constituents{:});                  
        end        
        
        function str = makeObjectName(obj)
            str = strings(size(obj));
            if obj.IsScalar
                str(1) = ...
                    obj.makeOneName(obj.Constituents);
            else            
                for idx = 1:numel(obj)
                    str(idx) = obj.makeOneName(obj.Constituents{idx});
                end
            end
        end
          
    end
    
    
    %% Internal methods
    methods(Access=private)
        function val = multiplyMonomialVectors(obj, lhs, rhs)
        % MULTIPLY Combine two monomial arrays
            len_lhs = numel(lhs);
            len_rhs = numel(rhs);
            if len_lhs == 0 || len_rhs ==0
                val = Symbolic.Monomial.empty(0, 1);
                return;
            end
            
            if numel(lhs) == 1
                if numel(rhs) == 1
                    opers = [lhs.Operators, rhs.Operators];
                    coefs = lhs.Coefficient * rhs.Coefficient;
                else
                    opers = cellfun(@(y) [lhs.Operators, y], ...
                                    rhs.Operators, ...
                                    'UniformOutput', false);
                    coefs = lhs.Coefficient .* rhs.Coefficient;
                end                
            elseif numel(rhs) == 1
                opers = cellfun(@(x) [x, rhs.Operators], ...
                                rhs.Operators, ...
                                'UniformOutput', false);
                coefs = lhs.Coefficient .* rhs.Coefficient;
            else
                opers = cellfun(@(x,y) [x, y], ...
                                repmat(lhs.Operators, len_rhs, 1), ...
                                repelem(rhs.Operators, len_lhs, 1), ...
                                'UniformOutput', false);                            
                coefs = repmat(lhs.Coefficient, len_rhs, 1) ...
                    .* repelem(rhs.Coefficient, len_lhs, 1);
            end

            % Construct monomials (find canonical form, etc.)
            val = Symbolic.Monomial(obj.Scenario, opers, coefs);
            
            % Clean-up polynomial
            val = obj.orderAndMerge(val);
        
        end
        function val = orderAndMerge(obj, monomials)
        % ORDERANDMERGE Sort monomials, and combine repeated elements.
        
            assert(isa(monomials,'Symbolic.Monomial'));
        
            % Trivial case: empty
            if numel(monomials) == 0
                val = Symbolic.Monomial.empty(0, 1);
                return;
            % Semi-trivial case: only one monomial (check it isn't zero!)
            elseif numel(monomials) == 1                
                [coef, m, mr, mi] = obj.Scenario.Prune(monomials.Coefficient);
                if m
                    val = Symbolic.Monomial.empty(0, 1);
                elseif mr || mi
                    val = Symbolic.Monomial(obj.Scenario, ...
                        monomials.Operators, coef);
                else
                    val = monomials;
                end
                return;
            end
            
            % Get Order
            [~, order] = sort(monomials.Hash);
            write_index = 0;
            
            % Merge construct
            op_cell = cell(0, 1);
            coefs = double.empty(0, 1);            
            last_hash = -1;
            
            for i = 1:numel(monomials)
                oid = double(order(i));
                next_hash = monomials.Hash(oid);
                if last_hash == next_hash
                    coefs(write_index, 1) = coefs(write_index) ...
                                + monomials.Coefficient(oid);
                else
                    write_index = write_index + 1;
                    coefs(write_index, 1) = monomials.Coefficient(oid);
                    op_cell{write_index, 1} = monomials.Operators{oid};
                end
                last_hash = next_hash;
            end
            
            % Identify and remove zeros
            [coefs, mask] = obj.Scenario.Prune(coefs);            
            if any(mask,'all')                
                if all(mask,'all')
                    val = Symbolic.Monomial.empty(0, 1);
                    return;
                else
                    coefs = coefs(mask);
                    op_cell = op_cell(mask);
                end
            end
                        
            % Make good polynomial
            val = Symbolic.Monomial(obj.Scenario, op_cell, coefs);
        end
        
        function makeFromOperatorCell(obj, input)
        %MAKEFROMOPERATORCELL Configure according to cell array input.
        
            % FIXME: Non-scalar input            
            if ~obj.IsScalar
                error("Not yet supported.");
            end
            
            obj.Constituents = Symbolic.Monomial.empty(1,0);
            for idx = 1:length(input)
                obj.Constituents(end+1) = ...
                    Symbolic.Monomial(obj.Scenario, ...
                        input{idx}{1}, input{idx}{2});                    
            end
        end
        
        function makeOperatorCell(obj)
        %MAKEOPERATORCELL Create cell description of polynomial.
            if obj.IsScalar
                obj.operator_cell = cell(1, length(obj.Constituents));
                for idx = 1:length(obj.Constituents)
                    obj.operator_cell{idx} = ...
                        {obj.Constituents(idx).Operators, ...
                         obj.Constituents(idx).Coefficient};
                end
                obj.done_oc = true;
            else
                error("Operator cell not supported for non-scalar polynomials.");
            end            
        end
       
        function makeSymbolCell(obj)
            
            if obj.IsScalar
                % No constituents -> {} 
                if isempty(obj.Constituents)
                    obj.symbol_cell = cell(1,0);
                    return;
                end

                % Not yet got symbols -> error
                if ~all([obj.Constituents.FoundSymbol])
                    error(obj.err_missing_symbol);
                end

                obj.symbol_cell = cell(1, length(obj.Constituents));
                for idx = 1:length(obj.Constituents)
                    obj.symbol_cell{idx} = {obj.Constituents(idx).SymbolId,...
                                            obj.Constituents(idx).Coefficient};
                    if (obj.Constituents(idx).SymbolConjugated)
                        obj.symbol_cell{idx}{end+1} = true;
                    end
                end
                obj.done_sc = true;
            else
                 error("Symbol cell not supported for non-scalar polynomials.");
            end            
        end
        
                
        function str = makeOneName(obj, constituents)
            if isempty(constituents)
                str = '0';
                return
            end
            
            str = constituents(1).ObjectName;
            for idx=2:numel(constituents)
                str = str + " + " + constituents(idx).ObjectName;
            end
        end      
    end
end

