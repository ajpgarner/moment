classdef(InferiorClasses={?MTKMonomial, ?MTKPolynomial}) VariantOutcome < handle
%VARIANTOUTCOME Outcome of a variant observable.
    
    %% Public properties
    properties(GetAccess=public, SetAccess=private)
        Scenario
        Id
        Index
    end
        
        
    %% Public dependent properties
    properties(Dependent, GetAccess = public, SetAccess = private)        
        % Operator sequence / polynomial representing outcome.
        Operators 
        % True if outcome is represented by a single operator sequence. 
        Singleton        
        % True if observable is a CV.
        ContinuousVariable 
    end
    
    %% Private cache
    properties(Access=private)
        ops = MTKPolynomial.empty(0,0);
    end
    
    %% Constructor
    methods(Access={?Inflation.Variant, ?Inflation.JointProbability})
        function obj = VariantOutcome(scenario, index)            
            obj.Scenario = scenario;
            obj.Index = reshape(uint64(index), 1, []);
            obj.Id = obj.Index(3);             
        end
    end
    
    %% Accessors
    methods
        function val = get.Operators(obj)
            if isempty(obj.ops)
                poly_spec = mtk('probability_table', 'full_sequences', ...
                                obj.Scenario.System.RefId, [], obj.Index);
                            
                obj.ops = ...
                    MTKPolynomial.InitFromOperatorCell(obj.Scenario, ...
                                                       poly_spec);
                obj.ops.ReadOnly = true;
            end
            val = obj.ops;
        end
        
        function val = get.Singleton(obj)
            the_ops = obj.Operators;
            val = the_ops.Constituents.IsScalar;
        end
        
        function val = get.ContinuousVariable(obj)
            val = obj.Scenario.Observables(obj.Index(1)).ContinuousVariable;
        end
    end
    
     %% Algebraic manipulation
    methods
        function val = mtimes(lhs, rhs)
        % TIMES Multiplication *
            
            % Check, for dominated LHS
            if ~isa(lhs, 'Inflation.VariantOutcome')
                this = rhs;
                other = lhs;
                this_on_left = false;
            else
                this = lhs;
                other = rhs;
                this_on_left = true;
            end
            
            % Treat as if a polynomial...
            if isnumeric(other)
                the_ops = this.Operators;
                if this_on_left
                    val = mtimes(the_ops, other);               
                else
                    val = mtimes(other, the_ops);
                end
                return;
            end
            
            % Special case: CV x CV/Polynomial
            if this.ContinuousVariable
                if isa(other, 'Inflation.VariantOutcome')
                    if this.Index(1) == other.Index(1)
                        this_expr = ...
                            this.Scenario.get(this.Index(1), this.Index(2)).ExplicitOutcomes;
                        other_expr = ...
                            this.Scenario.get(other.Index(1), other.Index(2)).ExplicitOutcomes;
                        
                        if this_on_left
                            val = mtimes(this_expr, other_expr);
                        else
                            val = mtimes(other_expr, this_expr);
                        end
                        return;
                    end
                elseif isa(other, 'MTKPolynomial') || isa(other, 'MTKMonomial')
                    if ~other.IsScalar
                        error("Can only combine scalar objects.");
                    end
                    cv_expr = this.Scenario.get(this.Index(1), this.Index(2)).ExplicitOutcomes;
                    if this_on_left
                        val = mtimes(cv_expr, other);
                    else
                        val = mtimes(other, cv_expr);
                    end
                    return;
                end
            end
            
            % Compose with another outcome to make joint outcome.
            if isa(other, 'Inflation.VariantOutcome')
                val = Inflation.JointProbability(this.Scenario, ...
                        Inflation.Variant.empty(1,0), [this, other]);
                return;
            end
            
            % Complain
            error("_*_ not defined between %s and %s", ...
                  class(lhs), class(rhs));
        end
        
        function val = times(lhs, rhs)          
        % TIMES Element-wise multiplication .*      
            
            % Check, for dominated LHS
            if ~isa(lhs, 'Inflation.VariantOutcome')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            % Treat as if a polynomial...
            if isnumeric(other)
                the_ops = this.Operators;
                val = times(the_ops, other);               
                return;
            end
            
            % Compose with another outcome to make joint outcome.
            if isa(other, 'Inflation.VariantOutcome')
                val = Inflation.JointProbability(this.Scenario, ...
                        Inflation.VariantOutcome.empty(1,0), [this, other]);
                return;
            end
            
            % Complain
            error("_.*_ not defined between %s and %s", ...
                  class(lhs), class(rhs));
        end
        
        function val = mpower(obj, index)
            if ~obj.ContinuousVariable
                error('mpower is only defined on variants that are continuous variables.');
            end

            cv_expr = this.Scenario.get(obj.Index(1), obj.Index(2)).ExplicitOutcomes;
            val = mpower(the_ops, double(index));
        end
        
    end    
end

