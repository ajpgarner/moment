classdef (InferiorClasses={?Locality.Outcome}) Measurement < handle
    %MEASUREMENT A collection of outcomes with assigned values
    
    %% Public properties
    properties(GetAccess=public, SetAccess=private)
        Scenario
        Id
        Index
        Name
        Outcomes
    end
    
    %% Public dependent properties
    properties(Dependent, GetAccess = public, SetAccess=private)
        ExplicitOutcomes
        ImplicitOutcomes
    end
    
    %% Private properties
    properties(Access=private)
        explicit = MTKMonomial.empty(1,0);
        implicit = MTKPolynomial.empty(1,0);
    end
        
    %% Error messages
    properties(Constant, Access = protected)
        err_overlapping_parties = ...
            "_*_ can only be used to form linear combinations of "...
            + "probabilities (i.e. all operands must be from different "...
            + "parties).";
    end
    
    %% Methods
    methods
        function obj = Measurement(scenario, party_index, mmt_index, ...
                                   name, num_outcomes)
            arguments
                scenario (1,1) LocalityScenario
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                name (1,1) string
                num_outcomes (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            import Locality.Outcome
            
            obj.Scenario = scenario;
            obj.Id = mmt_index;
            obj.Index = uint64([party_index, mmt_index]);
            obj.Name = name;
            
            
            % Check values, set default values
            if nargin <= 5
                if num_outcomes == 2
                    values = [+1, -1];
                else
                    values = 1:num_outcomes;
                end
            else
                if length(values) ~= num_outcomes
                    error("Number of outcomes must match number of supplied values")
                end
            end
            
            % Construct outcomes
            obj.Outcomes = Outcome.empty;
            for x = 1:num_outcomes
                obj.Outcomes(end+1) = Outcome(obj.Scenario, ...
                                              obj.Index(1), ...
                                              obj.Index(2), uint64(x));
            end
        end
    end
    
    %% Dependent methods
    methods
        function val = get.ExplicitOutcomes(obj)
            if ~obj.Scenario.HasMatrixSystem
                error("Explicit outcomes cannot be generated before matrix system is created.");
            end
            
            if isempty(obj.explicit)                
                [ops, hashes] = mtk('collins_gisin', 'sequences', ...
                             obj.Scenario.System.RefId, obj.Index);
                coefs = ones(size(hashes));
                try
                    [symbols, real_indices] = mtk('collins_gisin', 'symbols', ...
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
                                symbols, conj, real_indices, im_indices);
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
            if ~obj.Scenario.HasMatrixSystem
                error("Implicit outcomes cannot be generated before matrix system is created.");
            end
            
            if isempty(obj.implicit)
                poly_spec = mtk('probability_table', 'full_sequences',...
                    obj.Scenario.System.RefId, obj.Index);
                obj.implicit = ...
                    MTKPolynomial.InitFromOperatorCell(obj.Scenario, ...
                                                       poly_spec);
                obj.implicit.ReadOnly = true;
            end
            
            val = obj.implicit;            
        end
    end
    
    %% Conversion methods
    methods
        function val = MTKMonomial(obj)
            val = obj.ExplicitOutcomes;
        end
        
        function val = MTKPolynomial(obj)
            val = obj.ImplicitOutcomes;
        end
    end
    
    %% Probability methods
    methods      
        function val = Probability(obj, distribution, varargin)            
            val = Locality.make_explicit(obj.Scenario, ...
                obj.Index, uint64.empty(0,3), false, ...
                distribution, varargin{:});           
        end
    end
       
    %% Algebraic Manipulation
    methods
        function val = mtimes(lhs, rhs)
        % MTIMES Multiplication *
        
            % Check, for dominated LHS
            if ~isa(lhs, 'Locality.Measurement')
                this = rhs;
                other = lhs;
            else
                this = lhs;
                other = rhs;
            end
            
            % Compose measurements
            if isa(other, 'Locality.Measurement')
                val = Locality.JointProbability(this.Scenario, ...
                                                [this, other], ...
                                                Locality.Outcome.empty(1,0));                
                return;
            end
            
            % Compose measurement with outcome
            if isa(other, 'Locality.Outcome')
                val = Locality.JointProbability(this.Scenario, ...
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
            if ~isa(lhs, 'Locality.Measurement')
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
        
        function val = Correlator(this, other)
            if ~isa(other, 'Locality.Measurement')
                error("Correlator not defined for object of type %s", class(other));
            end
            
            joint_object = mtimes(this, other);
            val = joint_object.Correlator;           
        end
    end

end
