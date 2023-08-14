classdef Observable < handle
    %MEASUREMENT A classical measurement for causal network scenarios
    
    properties(SetAccess=private, GetAccess=public)
        Scenario
        Id
        OutcomeCount       
        Sources
    end
    
    properties(Dependent,SetAccess=private, GetAccess=public)
        % True if observable is a continuous variable
        ContinuousVariable
        
        Variants
        VariantCount
        TotalOperatorCount
        OperatorNames
        OperatorOffset
    end
            
    properties(Access = private)
        cached_op_names = string.empty(1,0);
        oper_offset = uint64(0);
        variants = Inflation.Variant.empty(0, 1);
    end
    
    %% Construction and initialization
    methods(Access={?Inflation.Observable,?InflationScenario})
        function obj = Observable(scenario, id, outcomes)

            % Validate parameters
            assert(nargin == 3)
            assert(isa(scenario, 'InflationScenario'));
            assert(numel(id) == 1)
            assert(numel(outcomes) == 1)
            id = uint64(id);
            outcomes = uint64(outcomes);
                        
            % Set values
            obj.Scenario = scenario;
            obj.Id = id;
            obj.OutcomeCount = outcomes;
            obj.Sources = Inflation.Source.empty(0,1);
        end
    
        function AddSource(obj, source)            
            % Validate
            assert(isa(source, 'Inflation.Source') && numel(source)==1);
            
            % Add to list
            obj.Sources(end+1) = source;
            
            % Sort, if multiple sources
            if numel(obj.Sources) > 1
                [~, order] = sort([obj.Sources.Id]);
                obj.Sources = obj.Sources(order);                
            end
            
            % Invalidate operator names
            obj.cached_op_names = string.empty(1,0);
        end
        
        function OtherObservableAdded(obj, ~)
            
            % Invalidate operator names
            obj.cached_op_names = string.empty(1,0);
        end
        
        function MakeVariants(obj, offset)
            obj.oper_offset = uint64(offset(1));
            
            variant_count = obj.VariantCount;
            obj.variants = Inflation.Variant.empty(0, 1);
            for idx=1:variant_count 
                obj.variants(end+1) = ...
                    Inflation.Variant(obj.Scenario, ...
                                      uint64([obj.Id, idx]));
            end
            
        end       
    end
    
    %% Derived information
    methods
        function val = get.ContinuousVariable(obj)
            val = obj.OutcomeCount == 0;
        end
        
        function val = get.VariantCount(obj)
            source_count = numel(obj.Sources);
            inflation = obj.Scenario.InflationLevel;
            val = power(inflation, source_count);
        end
        
        function val = get.Variants(obj)
            if ~obj.Scenario.HasMatrixSystem
                error("Variants not available until matrix system created.");
            end
            val = obj.variants;
        end
        
        function val = get.TotalOperatorCount(obj)
            variant_count = obj.VariantCount;
            if obj.OutcomeCount > 1
                val = uint64(variant_count * (obj.OutcomeCount-1));
            else
                val = uint64(variant_count); % CV, 1 op per variant
            end
        end
        
        function str = get.OperatorNames(obj)
            if isempty(obj.cached_op_names)
                obj.cached_op_names = makeOperatorNames(obj);
            end
            str = obj.cached_op_names;
        end
        
        function val = get.OperatorOffset(obj)
            if ~obj.Scenario.HasMatrixSystem
                error("Operator offset not defined until matrix system created.");
            end
            val = obj.oper_offset;
        end
    end
    
    %% Utility
    methods
        function index = VariantOffsetToIndex(obj, offset)
            assert(numel(offset) == 1);            
            offset = uint64(offset);
            sizes = uint64(ones(1, numel(obj.Sources))) ...
                        .* obj.Scenario.InflationLevel;
            index = MTKUtil.index_to_sub(sizes, offset);
        end
    end
    
    %% Private methods
    methods(Access=private)
        function str = makeOperatorNames(obj)
            % Get base name of observable, alphabetically.
            base_name = MTKUtil.alphabetic_index(obj.Id, true, false);
            
            % First, observable names
            if obj.OutcomeCount > 2
                obs_names = strings(1, obj.OutcomeCount-1);
                for odx = 1:(obj.OutcomeCount-1)
                    obs_names(odx) = sprintf("%s%d", base_name, (odx-1));
                end
            else
                obs_names = string(base_name);
            end
            
            % Do we need braces?
            add_brace = any([obj.Scenario.Observables.OutcomeCount] > 2);
            
            % Now, per outcome, per variant
            v_count = obj.VariantCount;
            if v_count > 1
                out_index = 1;
                str = strings(1, v_count*numel(obs_names));
                for vdx = 1:v_count
                    indices = obj.VariantOffsetToIndex(vdx) - 1;
                    if obj.Scenario.InflationLevel>= 10
                        var_str = join(string(indices), ',');
                    else
                        var_str = join(string(indices), '');
                    end
                    
                    for odx = 1:numel(obs_names)
                        if add_brace
                            str(out_index) = ...
                                sprintf("%s[%s]", obs_names(odx), var_str);
                        else
                            str(out_index) = ...
                                sprintf("%s%s", obs_names(odx), var_str);
                        end                        
                        out_index = out_index + 1;
                    end
                end
            else
                str = obs_names;
            end            
        end
    end
    
end

