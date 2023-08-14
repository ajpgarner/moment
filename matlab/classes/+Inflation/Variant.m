classdef (InferiorClasses={?Inflation.VariantOutcome}) Variant < handle
%OUTCOME Observable variant (i.e. inflated observable).
    properties(SetAccess = private, GetAccess = public)
        Scenario
        Id
        Index
        Outcomes
    end
    
    properties(Dependent, SetAccess=private, GetAccess=public)
        % True if observable is a continuous variable
        ContinuousVariable        
        SourceIndices
        ExplicitOutcomes
        ImplicitOutcomes
    end
    
    properties(Access=private)        
        explicit = MTKMonomial.empty(1,0);
        implicit = MTKPolynomial.empty(1,0);
    end
    
    % Constructor
    methods(Access={?Inflation.Observable})
        function obj = Variant(scenario, index)
            
            % Validate parameters
            assert(nargin == 2);
            assert(isa(scenario, 'InflationScenario'));
            assert(numel(index) == 2);
            index = reshape(uint64(index), 1, []);
            
            % Set values
            obj.Scenario = scenario;
            obj.Id = index(2);
            obj.Index = index;
            
            count = obj.Scenario.Observables(obj.Index(1)).OutcomeCount;
            
            if count > 0
                % Make normal outcome objects
                obj.Outcomes = Inflation.VariantOutcome.empty(0, 1);
                for odx = 1:count
                    obj.Outcomes(end+1) = ...
                        Inflation.VariantOutcome(obj.Scenario,...
                                                [obj.Index, odx]);
                end
            else
                % Make CV outcome object
                obj.Outcomes = Inflation.VariantOutcome(obj.Scenario, ...
                                                        [obj.Index, 1]);
            end
        end
    end
    
    % Convertors
    methods
        function val = MTKMonomial(obj)
            if ~obj.ContinuousVariable
                error("Only CV variants can be directly converted to monomials.");
            end
            val = obj.ExplicitOutcomes;
        end
    end

    % Accessors
    methods
        function val = get.ContinuousVariable(obj)
            val = obj.Scenario.Observables(obj.Index(1)).ContinuousVariable;
        end
        
        function val = get.SourceIndices(obj)
            obs = obj.Scenario.Observables(obj.Index(1));
            val = obs.VariantOffsetToIndex(obj.Index(2));
        end
        
        function val = get.ExplicitOutcomes(obj)
            if isempty(obj.explicit)
                [ops, hashes] = mtk('collins_gisin', 'sequences', ...
                    obj.Scenario.System.RefId, obj.Index);
                coefs = ones(size(hashes));
                try
                    [symbols, real_indices, is_aliased] ...
                        = mtk('collins_gisin', 'symbols', ...
                              obj.Scenario.System.RefId, obj.Index);
                    has_symbols = true;
                catch CGE
                    if isequal(CGE.identifier, 'mtk:missing_cg')
                        has_symbols = false;
                    else
                        rethrow(CGE);
                    end
                end
                if (has_symbols)
                    conj = false(size(hashes));
                    im_indices = zeros(size(hashes));
                    obj.explicit = ...
                        MTKMonomial.InitAllInfo(obj.Scenario, ...
                        ops, coefs, hashes, ...
                        symbols, conj, real_indices, im_indices, ...
                        is_aliased);
                else
                    obj.explicit = ...
                        MTKMonomial.InitDirect(obj.Scenario, ...
                        ops, coefs, hashes);
                end
                obj.explicit.ReadOnly = true;
            end
            
            val = obj.explicit;
        end
        
        function val = get.ImplicitOutcomes(obj)            
            if isempty(obj.implicit)
               % Special case: CV
                if obj.ContinuousVariable
                    obj.implicit = obj.Outcomes(1).Operators;
                else 
                    poly_spec = mtk('probability_table', 'full_sequences',...
                        obj.Scenario.System.RefId, obj.Index);
                    obj.implicit = ...
                        MTKPolynomial.InitFromOperatorPolySpec(obj.Scenario, ...
                        poly_spec);
                    obj.implicit.ReadOnly = true;
                end
            end
            
            val = obj.implicit;
        end
    end
    
    %% Algebraic Manipulation
    methods
        function val = mtimes(lhs, rhs)
        % MTIMES Multiplication *
        
            % Check, for dominated LHS
            if ~isa(lhs, 'Inflation.Variant')
                this = rhs;
                other = lhs;
                this_on_lhs = false;
            else
                this = lhs;
                other = rhs;
                this_on_lhs = true;
            end
            
            % Special case: if CV, take output and call again.
            if this.ContinuousVariable
                if this_on_lhs
                    val = mtimes(this.Outcomes(1), other);
                else
                    val = mtimes(other, this.Outcomes(1));
                end
                return;
            end

            % Compose variants
            if isa(other, 'Inflation.Variant')
                val = Inflation.JointProbability(this.Scenario, ...
                                                [this, other], ...
                                                Locality.Outcome.empty(1,0));                
                return;
            end

            % Compose variant with outcome
            if isa(other, 'Inflation.VariantOutcome')
                val = Inflation.JointProbability(this.Scenario, ...
                                                this, other);
                return;
            end

            % Complain
            error("_*_ not defined between %s and %s", ...
                  class(lhs), class(rhs));                 
        end
        
        function val = times(lhs, rhs)
        % TIMES Element-wise multiplication .*
            % Check, for dominated LHS
            if ~isa(lhs, 'Inflation.Variant')
                this = rhs;
                other = lhs;
                this_on_left = false;
            else
                this = lhs;
                other = rhs;
                this_on_left = true;
            end
            
            % Compose numerically
            if isnumeric(other)
                impl = this.ImplicitOutcomes;
                val = mtimes(impl, other);
                return;
            end
         
            % Complain
            error("_.*_ not defined between %s and %s", ...
                  class(lhs), class(rhs));
        end
        
        function val = plus(lhs, rhs)
             % Check, for dominated LHS
            if ~isa(lhs, 'Inflation.Variant')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            % Cast to implicit/explicit variables and call again
            if this.ContinuousVariable
                expl = this.ExplicitOutcomes;
                val = plus(expl, other);                
            else 
                impl = this.ImplicitOutcomes;
                val = plus(impl, other);
            end           
        end
                
        function val = minus(lhs, rhs)
            val = plus(lhs, -rhs);
        end
        
        function val = uminus(this)
             if this.ContinuousVariable
                expl = this.ExplicitOutcomes;
                val = uminus(expl);  
             else 
                impl = this.ImplicitOutcomes;
                val = uminus(impl);
             end
        end
        
        function val = mpower(obj, index)
            if ~obj.ContinuousVariable
                error('mpower is only defined on variants that are continuous variables.');
            end
            
            the_ops = obj.ExplicitOutcomes;
            val = mpower(the_ops, double(index));            
        end
        
        function val = Correlator(this, other)
            if ~isa(other, 'Inflation.Variant')
                error("Correlator not defined for object of type %s", class(other));
            end
            
            joint_object = mtimes(this, other);
            val = joint_object.Correlator;           
        end
        
        function val = Apply(obj, re_vals, ~)
        % APPLY Forward to Apply function of outcome polynomials.
            if obj.ContinuousVariable
                expl = obj.ExplicitOutcomes;
                val = expl.Apply(re_vals);
            else
                impl = obj.ImplicitOutcomes;
                val = impl.Apply(re_vals);
            end
        end
    end
end
    