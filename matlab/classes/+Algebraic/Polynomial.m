classdef (InferiorClasses={?Algebraic.Monomial,?Algebraic.Zero}) ...
        Polynomial < Abstract.ComplexObject
%POLYNOMIAL A polynomial expression of operators (or their moments).
    
    properties
        Constituents = Algebraic.Monomial.empty(1,0)
    end
    
    properties(GetAccess=public, SetAccess=private)
        OperatorCell = cell(1, 0);
    end
        
    properties(Dependent)
        SymbolCell
        IsZero
    end
    
    properties(Access=private)
        done_sc = false;
        symbol_cell = cell(1, 0);
        done_oc = false;
    end
    
    methods
        function obj = Polynomial(setting, constituents)
            arguments
                setting (1,1) Abstract.Scenario
                constituents (1,:) 
            end
            obj = obj@Abstract.ComplexObject(setting);
            obj.ObjectName = "Polynomial";
            
            if isempty(constituents) 
                obj.Constituents = Algebraic.Monomial.empty(1,0);
            elseif isa(constituents, 'Algebraic.Monomial')
                for other = constituents
                    obj.checkSameScenario(other);
                end                        
                obj.Constituents = constituents;
            elseif isa(constituents, 'cell')
               obj.makeFromOperatorCell(constituents);
            else
                error("Polynomial must be initialized from array of monomials, or a cell array.")
            end
               
            obj.orderAndMerge();
            obj.makeObjectName();
            obj.makeOperatorCell();
        end
        
    end
    
    %% Dependent variables
    methods
        function val = get.IsZero(obj)
            val = isempty(obj.Constituents);
        end
        
        function val = get.SymbolCell(obj)
            if ~obj.done_sc
                obj.makeSymbolCell();
                if ~obj.done_sc
                    error("Not yet got symbol cell.");
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
                
                if other == 0 
                    val = Algebraic.Zero(this.Scenario);
                else                 
                    new_coefs = Algebraic.Monomial.empty(1,0);
                    for i = 1:length(this.Constituents)
                        old_m = this.Constituents(i);
                        new_coefs(end+1) = ...
                            Algebraic.Monomial(this.Scenario, ...
                                           old_m.Operators, ...
                                           double(old_m.Coefficient * other));
                    end
                    val = Algebraic.Polynomial(this.Scenario, new_coefs);
                end
            elseif isa(other, 'Algebraic.Zero')
                this.checkSameScenario(other);
                val = Algebraic.Zero(this.Scenario);                
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
                val = Algebraic.Zero(this.Scenario);
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
                if abs(other) < 2*eps(1)
                    val = +this;
                    return
                end
                
                other = Algebraic.Monomial(this.Scenario, [], double(other));
            elseif  ~isa(other, 'Algebraic.Zero') && ...
                    ~isa(other, 'Algebraic.Monomial') && ...
                    ~isa(other, 'Algebraic.Polynomial')
                error("_+_ not defined between " + class(lhs) ...
                        + " and " + class(rhs));
            end
            
            % Check objects are from same scenario
            this.checkSameScenario(other);
                        
           
            if isa(other, 'Algebraic.Zero')
                val = Algebraic.Polynomial(this.Scenario, this.Constituents);
            elseif isa(other, 'Algebraic.Monomial')
                components = horzcat(this.Constituents, other);
                val = Algebraic.Polynomial(this.Scenario, components);
            elseif isa(other, 'Algebraic.Polynomial')
                if ~other.IsZero
                    components = horzcat(this.Constituents, other.Constituents);
                    val = Algebraic.Polynomial(this.Scenario, components);
                end
            else
                error(['Assertion failed: ',...
                       'other should be Zero, Monomial or Polynomial.']);
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
                if abs(val.Coefficient) < 2*eps(1)
                    val = Algebraic.Zero(obj.Scenario);
                end
            elseif obj.IsZero
                val = Algebraic.Zero(obj.Scenario);
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
    
                
    %% Apply substitution rules
    methods        
        function val = ApplyRules(obj, rulebook)
        % APPLYRULES Transform moments of matrix according to rulebook.
        %
        % Effectively applies rules to each constituent matrix in turn.
        % 
            arguments
                obj (1,1) Algebraic.Polynomial
                rulebook (1,1) MomentRuleBook
            end
            
            % Scenarios must match
            if (rulebook.Scenario ~= obj.Scenario)
                error(obj.err_mismatched_scenario);
            end
            
            % Get transformed version of polynomial
            as_symbol_cell = obj.SymbolCell();
            output_sequences = mtk('apply_moment_rules', ...
                obj.Scenario.System.RefId, rulebook.RuleBookId, ...
                'output', 'sequences', as_symbol_cell);
            
            % Construct new, transformed polynomial
            val = Algebraic.Polynomial(obj.Scenario, output_sequences);
            
            % Degrade to monomial if only single element.
            if length(val.Constituents) == 1
                val = val.Constituents(1);
            end
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
            
            %%FIXME: that's a hack, should be done properly at some point
            real_mask = abs(obj.real_coefs) <= 2*eps(1);
            im_mask = abs(obj.im_coefs) <= 2*eps(1);
            obj.real_coefs(real_mask) = 0;
            obj.im_coefs(im_mask) = 0;
            
        end
    end
    
    
    %% Internal methods
    methods(Access=private)
        function orderAndMerge(obj)
        % ORDERANDMERGE Sort monomials, and combine repeated elements.
        
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
            nz_mask = abs([nc(:).Coefficient]) >= 2*eps(1);
            obj.Constituents = nc(nz_mask);
        end
        
        function makeFromOperatorCell(obj, input)
        %MAKEFROMOPERATORCELL Configure according to cell array input.
            obj.Constituents = Algebraic.Monomial.empty(1,0);
            for idx = 1:length(input)
                obj.Constituents(end+1) = ...
                    Algebraic.Monomial(obj.Scenario, ...
                        input{idx}{1}, input{idx}{2});                    
            end
        end
        
        function makeOperatorCell(obj)
        %MAKEOPERATORCELL Create cell description of polynomial.
            obj.OperatorCell = cell(1, length(obj.Constituents));
            for idx = 1:length(obj.Constituents)
                obj.OperatorCell{idx} = ...
                    {obj.Constituents(idx).Operators, ...
                     obj.Constituents(idx).Coefficient};
            end            
        end
            
        
        function makeObjectName(obj)
        % MAKEOBJECTNAME Create human-readable representation of polynomial.
            
            if isempty(obj.Constituents)
                obj.ObjectName = 'Polynomial "0"';
                return
            end
            obj.ObjectName = 'Polynomial "' + obj.Constituents(1).ObjectName;
            if length(obj.Constituents) == 1  
                obj.ObjectName = obj.ObjectName + '"';
                return;
            end
            for idx=2:length(obj.Constituents)
                obj.ObjectName = obj.ObjectName + " + " ...
                                 + obj.Constituents(idx).ObjectName;
            end
            obj.ObjectName = obj.ObjectName + '"';
        end
        
        function makeSymbolCell(obj)
            % No constituents -> {} 
            if isempty(obj.Constituents)
                obj.symbol_cell = cell(1,0);
                return;
            end
            
            % Not yet got symbols -> error
            if ~all([obj.Constituents.FoundSymbol])
                error("Constituent terms not yet resolved to symbols.");
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
        end
    end
end

