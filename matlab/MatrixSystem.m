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
            
            % Check if settingsParams is Scenario, or cell array, then call
            % make_matrix_system to load setting into C++ code.
            if isa(settingParams, 'cell')
                % Unpack cell into arguments
                obj.RefId = npatk('make_matrix_system', settingParams{:});                
            elseif isa(settingParams, 'Scenario')
                % Supply setting object directly
                obj.RefId = npatk('make_matrix_system', settingParams);
            else
                error(['First argument must be either a Scenario ',...
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
                obj.SymbolTable = npatk('get_symbol_table', obj.RefId);
                has_new_symbols = true;
            else
                existing_id = uint64(length(obj.SymbolTable));
                new_symbols = npatk('get_symbol_table', ...
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
                mustBeValidVariableName(im_name);
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
        function varargout = yalmipCreateVars(obj)
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
            
            varargout = cell(1, nargout);
            varargout{1} = sdpvar(obj.RealVarCount, 1);
            if export_imaginary
                varargout{2} = sdpvar(obj.ImaginaryVarCount, 1);
            end
        end
    end
    
end

