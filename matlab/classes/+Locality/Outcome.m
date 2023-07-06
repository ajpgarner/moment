classdef Outcome
    %OUTCOME Measurement outcome
    properties(SetAccess={?LocalityScenario}, GetAccess = public)
        Scenario
        Index
        Id
    end
    
        
    %% Public dependent properties
    properties(Dependent, GetAccess = public, SetAccess = private)
        Operators % Operator sequence / polynomial representing outcome.
        Singleton % True if outcome is represented by a single operator sequence.        
    end
    
    %% Private cache
    properties(Access=private)
        ops = MTKPolynomial.empty(0,0);
    end
        
    %% Constructor
    methods
        function obj = Outcome(setting, party_index, ...
                               mmt_index, outcome_index)
            arguments
                setting (1,1) LocalityScenario
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                outcome_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            
            obj.Scenario = setting;
            obj.Id = outcome_index;
            obj.Index = uint64([party_index, mmt_index, outcome_index]);            
        end
    end
    
    %% Accessors
    methods
        function val = get.Operators(obj)
            if isempty(obj.ops)
                poly_spec = mtk('probability_table','full_sequences', ...
                                obj.Scenario.System.RefId, [], obj.Index);
                            
                obj.ops = ...
                    MTKPolynomial.InitFromOperatorCell(obj.Scenario, ...
                                                       poly_spec);
            end
            val = obj.ops;
        end
        
        function val = get.Singleton(obj)
            the_ops = obj.Operators;
            val = the_ops.Constituents.IsScalar;
        end
        
    end
    
    %% Algebraic manipulation
    methods
        function val = mtimes(lhs, rhs)
        % TIMES Multiplication *
            
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
            
            % Compose with another outcome to make joint outcome.
            if isa(other, 'Locality.Outcome')
                val = Locality.JointProbability(this.Scenario, ...
                        Locality.Measurement.empty(1,0), [this, other]);
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
            if isa(other, 'Locality.Outcome')
                val = Locality.JointProbability(this.Scenario, ...
                        Locality.Measurement.empty(1,0), [this, other]);
                return;
            end
            
            
            % Complain
            error("_.*_ not defined between %s and %s", ...
                  class(lhs), class(rhs));
        end
        
    end    
end