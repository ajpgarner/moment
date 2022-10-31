classdef MatrixSystem < handle
    %MATRIXSYSTEM A matrix of operator products. Wraps a reference to a
    % MatrixSystem class stored within npatk.
    
    properties(SetAccess = private, GetAccess = public)
        RefId = uint64(0)        
        SymbolTable = struct.empty;
        ProbabilityTable = struct.empty;
        RealVarCount = uint64(0)
        ImaginaryVarCount = uint64(0) 
    end
    
    properties(Access = private)
        probability_table = struct.empty;
        max_prob_len = uint64(0);
    end
    
    methods
        %% Constructor
        function obj = MatrixSystem(settingParams)
            % Check if settingsParams is LocalityScenario, or cell array, 
            % then call appropriate function to load setting into C++.
            if isa(settingParams, 'cell')
                % Unpack cell into arguments
                obj.RefId = npatk('new_locality_matrix_system', settingParams{:});
            elseif isa(settingParams, 'LocalityScenario')
                % Unpack setting into arrays
                obj.RefId = npatk('new_locality_matrix_system', ...
                                  length(settingParams.Parties), ...
                                  settingParams.MeasurementsPerParty, ...
                                  settingParams.OutcomesPerMeasurement);
            else
                error(['First argument must be either a LocalityScenario ',...
                    'object, or a cell array of parameters.']);
            end
            
            obj.UpdateSymbolTable();
                        
        end
        
        function val = MakeMomentMatrix(obj, level)
            arguments
                obj (1,1) MatrixSystem
                level (1,1) {mustBeNonnegative, mustBeInteger} = 1
            end
            val = MomentMatrix(obj, level);
        end
        
        function val = UpdateSymbolTable(obj)
            has_new_symbols = false;
            if isempty(obj.SymbolTable)
                obj.SymbolTable = npatk('symbol_table', obj.RefId);
                has_new_symbols = true;
            else
                existing_id = uint64(length(obj.SymbolTable));
                new_symbols = npatk('symbol_table', ...
                                        obj.RefId, existing_id);
                if ~isempty(new_symbols)
                    has_new_symbols = true;
                    obj.SymbolTable = [obj.SymbolTable, new_symbols];
                end                
            end
            
            % Extra processing, as new symbols added:
            if has_new_symbols
                obj.RealVarCount = max([obj.SymbolTable.basis_re]);
                obj.ImaginaryVarCount = max([obj.SymbolTable.basis_im]);                
                obj.ProbabilityTable = npatk('probability_table', obj.RefId);
            end
            
            val = obj.SymbolTable;
        end
        
        
        %% Destructor
        function delete(obj)
            if obj.RefId ~= 0
                try
                    npatk('release', 'matrix_system', obj.RefId);
                catch ME
                    fprintf(2, "Error deleting MatrixSystem: %s\n", ...
                        ME.message);
                end
            end
        end        
    end
    
                  
    %% Accessors for probability table
    methods
        function val = get.ProbabilityTable(obj)
            % PROBABILITYTABLE A struct-array indicating how each
            %   measurement outcome can be expressed in terms of real
            %   basis elements (including implied probabilities that do not
            %   directly exist as operators in any moment matrix).
            if (isempty(obj.probability_table))
                obj.probability_table = npatk('probability_table', ...
                                              obj.RefId);
            end
            val = obj.probability_table;
        end
        
        function result = MeasurementCoefs(obj, indices)
            arguments
                obj (1,1) MatrixSystem
                indices (:,2) uint64
            end            
            parties = indices(:, 1);
            if length(parties) ~= length(unique(parties))
                error("Measurements must be from different parties.");
            end
            
            result = npatk('probability_table', ...
                           obj.RefId, indices);
        end
    end
    
    %% CVX Methods
    methods
        function cvxCreateVars(obj, real_name, im_name)
            % Create SDP variables associated with MatrixSystem.
            % Due to the design of CVX eschewing functional programming,
            % real_name (resp. im_name) should be set to the name desired
            % in the 
            arguments
                obj (1,1) MatrixSystem
                real_name (1,:) char {mustBeValidVariableName}
                im_name (1,:) char = char.empty
            end
            
            % Check if exporting real, or real & imaginary
            export_imaginary = ~isempty(im_name);
            if export_imaginary
             	if ~isvarname(im_name)
             		error("Invalid variable name.");
             	end
            end
            
            % Get handle to CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            % Always prepare real variables
            rvname = strcat(real_name, '(obj.RealVarCount)');            
            rv = variable(rvname);
            assignin('caller', real_name, rv);
                        
            % Sometimes prepare imaginary variables
            if export_imaginary
                ivname = strcat(im_name, '(obj.ImaginaryVarCount)');                
                iv = variable(ivname);
                assignin('caller', im_name, iv);
            end
        end
    end
    
    
    %% Yalmip Methods
    methods
        function [var_A, var_B] = yalmipCreateVars(obj)
            % Creates SDP variables associated with MatrixSystem.
            % First output corresponds to real components. Second output 
            % (if requested) correponds to imaginary components.
            arguments
                obj (1,1) MatrixSystem
            end
            
            if nargout == 1
                export_imaginary = false;
            elseif nargout == 2
                export_imaginary = true;
            else
                error("One or two outputs expected.");
            end
           
            var_A = sdpvar(double(obj.RealVarCount), 1);
            if export_imaginary
                var_B = sdpvar(double(obj.ImaginaryVarCount), 1);
            end
        end
    end
    
end

