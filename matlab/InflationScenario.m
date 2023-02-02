classdef InflationScenario < Scenario
    %INFLATIONSCENARIO Classical observables with hidden classical sources.
    %
    
    properties(GetAccess = public, SetAccess = protected)
        InflationLevel
        Observables = Inflation.Observable.empty(1,0)
        Sources = Inflation.Source.empty(1,0)
    end
    
    properties(Dependent, GetAccess = public)
        OutcomesPerObservable
        ObservablesFromEachSource
    end
    
    
    %% Construction and initialization
    methods
        function obj = InflationScenario(inf_level, observables, sources)                        
            % Superclass c'tor
            obj = obj@Scenario();
            
            % Set inflation level
            if nargin>=1
                obj.InflationLevel = uint64(inf_level);
            else 
                obj.InflationLevel = 1;
            end
            
            % Add observables
            if nargin>=2
                for num_outcomes = observables
                    obj.AddObservable(num_outcomes);
                end
            end
            
            % Add sources
            if nargin>=3
                if ~iscell(sources)
                    error("Source list should be provided as cell array");
                end
                for sIndex = 1:length(sources)
                    obj.AddSource(sources{sIndex});                    
                end
            end
            
        end
        
        function AddObservable(obj, outcomes)
            arguments
                obj (1,1) InflationScenario
                outcomes (1,1) uint64
            end
            % Check not locked.
            obj.errorIfLocked();
            
            % Add to end of sources
            next_id = length(obj.Observables)+1;           
            obj.Observables(end+1) = Inflation.Observable(obj, next_id, ...
                                                          outcomes);
        end
       
        function AddSource(obj, targets)
            arguments
                obj (1,1) InflationScenario
                targets (1,:) uint64
            end
            % Check not locked.
            obj.errorIfLocked();
            
            % Remove duplicates and sort
            targets = unique(targets);
            
            % Check range
            max_target = length(obj.Observables);
            for tindex = 1:length(targets)
                if (targets(tindex) < 1) || (targets(tindex) > max_target)
                    error('Target "' + targets(tindex) + '" out of range:');
                end
            end
                
            % Add to end of targets
            next_id = length(obj.Sources)+1;
            obj.Sources(end+1) = Inflation.Source(obj, next_id, targets);           
        end
        
        function val = Clone(obj)
            arguments
                obj (1,1) InflationScenario
            end
            % Clone InflationScenario
            val = InflationScenario(obj.InflationLevel, ...
                                    obj.OutcomesPerObservable, ...
                                    obj.ObservablesFromEachSource);            
        end
    end
    
    
    methods
        function val = ObservablesToOperators(obj, array)
            % Translate observable IDs into operator IDs
            arguments
                obj (1,1) InflationScenario
                array (:,:) uint64
            end
            the_shape = size(array);
            array_row = array(:);            
            val = uint64(zeros(1, length(array_row)));
            
            for index = 1:length(array_row)
                probe_index = array_row(index);
                if probe_index > length(obj.Observables)
                    error("Observable out of bounds.");
                end
                val(index) = obj.Observables(probe_index).OperatorOffset;
            end
                        
            val = reshape(val, the_shape);            
        end
        
        function val = ObservablesToSymbols(obj, input_list)
            % Translate observable IDs into symbol IDs
            arguments
                obj (1,1) InflationScenario
                input_list (1,:) cell
            end
            for index = 1:length(input_list)
                input_list{index} = ...
                    obj.ObservablesToOperators(input_list{index});
            end
            
            result = mtk('symbol_table', obj.System.RefId, input_list);
            val = [result.symbol];
        end
    end
    
    %% Unique matrix types
    methods
        function val = MakeExtendedMomentMatrix(obj, level, extensions)
            % Sanitize input
            level = uint64(level);
            if nargin <= 3
                extensions = 'auto';
            else
                extensions = uint64(extensions);
            end
            % Call MTK to create
            [index, dimension] = ...
               mtk('extended_matrix', obj.System.RefId, level, extensions);
               
            % Make wrapper object            
            val = OperatorMatrix(obj.System, index, dimension);
            
            % Symbol table update
            obj.System.UpdateSymbolTable();
        end
    end
    
    %% Get assignments
    methods
        function val = GetAssignments(obj, param_A, param_B)
            if nargin < 3
                val = mtk('make_explicit', obj.System.RefId, param_A);
            else
                val = mtk('make_explicit', obj.System.RefId, ...
                          param_A, param_B);
            end
        end
    end
        
    %% Overloaded accessor: MatrixSystem
    methods
        function val = System(obj)
            arguments
                obj (1,1) InflationScenario
            end
            
            % Make matrix system, if not already generated
            if isempty(obj.matrix_system)
                obj.matrix_system = Inflation.InflationMatrixSystem(obj);
            end
            
            val = obj.matrix_system;
        end     
    end
        
    %% Causal network / inflation accessors and information
    methods
       function val = get.OutcomesPerObservable(obj)
           val = [obj.Observables.OutcomeCount];
       end
        
       function val = get.ObservablesFromEachSource(obj)
           val = {obj.Sources.TargetIndices};
       end
    end
 
    %% Friend/interface methods
    methods(Access={?Scenario,?MatrixSystem})
        % Query for a matrix system
        function ref_id = createNewMatrixSystem(obj)
            arguments
                obj (1,1) InflationScenario
            end
            [ref_id, canObs] = mtk('new_inflation_matrix_system', ...
                                   obj.OutcomesPerObservable, ...
                                   obj.ObservablesFromEachSource, ...
                                   obj.InflationLevel);
            for index = 1:length(obj.Observables)
                obj.Observables(index).OperatorOffset = canObs(index)+1;
            end
        end
    end
      
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) InflationScenario
                mm (1,1) MomentMatrix
            end            
        end
    end
end

