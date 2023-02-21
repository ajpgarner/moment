classdef InflationScenario < Abstract.Scenario
%INFLATIONSCENARIO Classical observables with hidden classical sources.
%
% The inflation setting is a type of causal-compatibility testing scenario, 
% composed of classical observables made by spatially-disjoint agents, 
% whose statistics are directly reported, and sources that are not directly 
% observable, but can exert causal influence on the observables. 
% 
% The operator strings generated correspond to moments of the observables.
% Operators from different observables (including inflationary copies of 
% the same observable) commute with each other. An observable can have a 
% finite number of outcomes, or be set as a 'continuous variable' object. 
% When the observable has N outcomes, then N-1 orthonormal projectors are
% generated satisfying Xi Xi = Xi and Xi Xj = 0 when i != j. If the 
% observable is a continuous variable, only one fundamental operator will 
% be generated for it, but it will not be projective such that higher order 
% moments (e.g. X^2, X^3) can appear.
%
% Inflating the scenario to inflation level M automatically generates new
% sources and observables according to the following: first, M copies of
% each source are created. Then, for each observable connected to s sources 
% in the uninflated scenario, M^s variant observables are created,
% corresponding to an observable affected by each permutation of variant 
% sources. [See: https://doi.org/10.1515/jci-2017-0020, especially figures 
% 1 and 2].
%
% The causal influences affect the moment matrices of the scenario by 
% imposing or disallowing factorization of moments. For example, if there 
% is a source X connecting observables A and B, then <AiBj> cannot be 
% further simplified, but if no such X connects the sources, then <AiBj> = 
% <Ai><Bj>.
%
% In this inflation scenario, the Moment toolkit allows for:
%  1. The identification of all such factorization relationships between 
%     observables.
%  2. (Partial) substitution of known quantities into factored strings. For
%     example, if A has no causal link to B and C, but B and C share a
%     common source, and if it is known that <Ai> = 0.4, then <AiBjCk> can
%     be replaced with 0.4<BjCk>.
%  3. The scalar extension of a moment matrix with additional terms, to 
%     impose (relaxations of) the factorization constraints.
% 
% EXAMPLES:
%       /examples/cvx_inflation_triangle.m
%       /examples/inflation_CV.m
%       /examples/yalmip_inflation_triangle.m
%

    properties(GetAccess = public, SetAccess = protected)
        InflationLevel % The number of 'inflated' copies of each source.
        % Observable measurements from disjoint agents.
        Observables = Inflation.Observable.empty(1,0)
        % Classical hidden variables.
        Sources = Inflation.Source.empty(1,0)
    end
    
    properties(Dependent, GetAccess = public)
        % Number of outcomes each observable has.
        OutcomesPerObservable 
        % The observables connected to each source.
        ObservablesFromEachSource 
    end
    
    %% Construction and initialization
    methods
        function obj = InflationScenario(inf_level, observables, sources)
        % Constructs an inflation causal-compatibility scenario.
        % 
        % PARAMS:
        %   inf_level   - The inflation level (1 = uninflated)
        %   observables - List of observables, by number of outputs.
        %                 Choose '0' outputs for continuous variable
        %   sources     -  Cell array of sources, each defined by an array 
        %                  of indices of the influencced observables.
        %
        % Example: CV pair, linked by source, uninflated:
        %   setting = InflationScenario(1, [0 0], {[1 2]})
        %
        % Example: Triangle scenario with bipartite observables, inflated 
        %          to level 2:
        %   setting = InflationScenario(2, [2 2 2], {[1 2], [2 3], [1,3]}})
        %
        % Example: Empty uninflated setting (to be later defined with 
        %          AddObservable / AddSource methods).
        %   setting = InflationScenario(1)
        %
        % See also: AddObservable, AddSource
        %
            arguments
                inf_level (1,1) uint64 = 1
                observables (1,:) uint64 = uint64.empty(1,0);
                sources (1,:) cell = cell(1,0)
            end

            % Call Superclass c'tor
            obj = obj@Abstract.Scenario();
            
            % Save inflation level
            obj.InflationLevel = inf_level;
            
            % Add observables
            if ~isempty(observables)
                for num_outcomes = observables
                    obj.AddObservable(num_outcomes);
                end
            end
            
            % Add sources
            if ~isempty(sources)
                for sIndex = 1:length(sources)
                    obj.AddSource(sources{sIndex});                    
                end
            end
        end

        function AddObservable(obj, outcomes)
        % ADDOBSERVABLE Add another observable to the scenario.
        % Will throw an error if the matrix system has already been
        % generated.
        %
        % PARAMS:
        %   outcomes - The number of outcomes the newly added observable
        %              has (set to 0 for continuous variable).
        %        
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
        % ADDSOURCE Add another hidden classical source to the scenario.
        %
        % PARAMS:
        %   targets - An array indexing the observables influenced by this
        %             source.
        %
        % Will throw an error if the matrix system has already been
        % generated.
        %
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
        % CLONE Makes a copy of the scenario.       
            arguments
                obj (1,1) InflationScenario
            end
            val = InflationScenario(obj.InflationLevel, ...
                                    obj.OutcomesPerObservable, ...
                                    obj.ObservablesFromEachSource);            
        end
    end
    
    methods
        function val = ObservablesToOperators(obj, array)
        % OBSERVABLESTOOPERATORS Translate observable indices into operator ids.
        %
        % PARAMS:
        %   array - Scalar, vector or matrix of observable indices.
        %
        % RETURNS:
        %   An object of the same shape as array, but with every index
        %   substituted by the number of the first fundamental operator
        %   associated with the observable. 
        %
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
        % OBSERVABLESTOSYMBOLS Retrieve symbol data on supplied observables.
        %
        % PARAMS:
        %   input_list - Cell array of observables and joint observable strings.
        %
        % RETURNS:
        %   List of symbol IDs in the same order as input_list,
        %   corresponding to the symbol representing the first operator
        %   associated with the specified observable.
        %        
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
        % MAKEEXTENDEDMOMENTMATRIX Create moment matrix with scalar extensions.
        %
        % PARAMS:
        %   level - The level of moment matrix to generate.
        %   extensions - Either an array of symbols that will be used to 
        %                extend the moment matrix, or the word 'auto' to 
        %                use the instrinsic factorization relationships 
        %                to suggest extensions.
        %
        arguments
            obj (1,1) InflationScenario
            level (1,1) uint64
            extensions = 'auto';
        end
        
            % Sanitize inputs
            level = uint64(level);
            if isnumeric(extensions)
                extensions = uint64(extensions);
            elseif ~strcmp(extensions, 'auto')
                error(['Extensions should either be an array of '...
                       'symbols, or the word "auto"']);                
            end
            
            % Call MTK to create
            [index, dimension] = ...
               mtk('extended_matrix', obj.System.RefId, level, extensions);
               
            % Make wrapper object            
            val = OpMatrix.OperatorMatrix(obj.System, index, dimension);
            
            % Symbol table update
            obj.System.UpdateSymbolTable();
        end
    end
    
    %% Get assignments
    methods
        function val = GetAssignments(obj, param_A, param_B)
        % GETASSIGNMENTS Return symbolic assignments that impose a probability distribution.
        % 
        % SYNTAX
        %  1. v = setting.GetAssignments([probabilities])
        %  2. v = setting.GetAssignments([obs. index], [probabilities])
        %  3. v = setting.GetAssignments([obs. index, var. index], [probabilities])
        %  4. v = setting.GetAssignments([obs. index A; obs. index B; ...],...
        %                                 [probabilities])
        %  5. v = setting.GetAssignments([obs. index A, var. index A; ...
        %                                 obs. index B, var. index B], [probabilities])
        %
        % PARAMS (Syntax 1)
        %   param_A - The probability distribution over the joint
        %             measurement of the first variant of all observables.
        %
        % PARAMS (Syntax 2-5)
        %   param_A - An array specifying the observable/joint observable.
        %             Each row represents an observable. The first column
        %             gives the observable number. The second column, if 
        %             provided, specifies which inflationary variant should
        %             be used.
        %   param_B - The joint probability distribution over the
        %             measurement defined by param_A.
        %
        % RETURNS
        %   A cell array of cell array pairs. In each pair, the first 
        %   element is a symbol ID, the second element the numeric value.
        %   These are provided in a format that could be input into
        %   OpMatrix.OperatorMatrix.ApplyValues.
        %
        % See also: OPMATRIX.OPERATORMATRIX.APPLYVALUES
        %

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
        % SYSTEM Gets MatrixSystem object associated with scenario.
        %
        % Will generate an InflationMatrixSystem if it has not yet been
        % created.
        %
        % RETURN:
        %   An Inflation.InflationMatrixSystem object.
        %
        % See also: Inflation.InflationMatrixSystem, MatrixSystem
        %
        arguments
            obj (1,1) InflationScenario
        end
            
            % Make matrix system, if not already generated
            if isempty(obj.matrix_system)
                obj.matrix_system = Inflation.InflationMatrixSystem(obj);
            end
            
            % Return handle
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
 
    %% Virtual methods
    methods(Access={?Abstract.Scenario,?MatrixSystem})        
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
                mm (1,1) OpMatrix.MomentMatrix
            end            
        end
    end
end

