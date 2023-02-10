classdef MatrixSystem < handle
    %MATRIXSYSTEM A matrix of operator products. 
    % Wraps a reference to a MatrixSystem class stored within mtk.
    
    properties(SetAccess = private, GetAccess = public)
        RefId = uint64(0)        
        SymbolTable = struct.empty;
        RealVarCount = uint64(0)
        ImaginaryVarCount = uint64(0) 
    end
    
    properties(Access = private)
        max_prob_len = uint64(0);
    end
    
    properties(Constant, Access = protected)
        err_bad_scenario_type = [
            'First cell of array argument must be one of "algebraic",',...
            '"locality", "imported" or "inflation"'];
        
        err_bad_ctor_arg = ['Argument must be either a Scenario class,',...
                            ' or a cell array of parameters.'];
    end  
    
    
    methods
        %% Constructor
        function obj = MatrixSystem(args)
            % Checks if args is a Scenario, or cell array, then calls
            % appropriate MEX function to initiate setting.
            % End-users are encouraged not to invoke this manually, but
            % prefer automating it through a [System-Type]Scenario class.
            
            if isa(args, 'Abstract.Scenario')
                % Invoke Scenario's own MatrixSystem creation function
                obj.RefId = args.createNewMatrixSystem();
            elseif isa(args, 'cell')
                if length(args) < 1
                    error(obj.err_bad_scenario_type);
                end
                
                % Make sure head is a string
                head = args{1};
                if ischar(head)
                    head = string(head);
                end
                if ~isstring(head) || ~isscalar(head)
                    error(obj.err_bad_scenario_type);
                end

                % Invoke appropriate MTK system creation
                switch(lower(head))
                    case "algebraic"
                        args{1} = 'new_algebraic_matrix_system';
                    case "locality"
                        args{1} = 'new_locality_matrix_system';
                    case "imported"
                        args{1} = 'new_imported_matrix_system';
                    case "inflation"
                        args{1} = 'new_inflation_matrix_system';
                    otherwise
                        error(obj.err_bad_scenario_type);
                end
                
                % Invoke MTK to create a matrix system
                obj.RefId = mtk(args{:});
            else
                error(obj.err_bad_ctor_arg);
            end
            
            obj.UpdateSymbolTable();
        end
    end
    
    %% Operator matrices
    methods    
        function val = MakeMomentMatrix(obj, level)
            arguments
                obj (1,1) MatrixSystem
                level (1,1) {mustBeNonnegative, mustBeInteger} = 1
            end
            val = MomentMatrix(obj, level);
        end
                
        function varargout = List(obj)
            the_list = mtk('list', obj.RefId);
            if nargout == 1
                varargout{1} = the_list;
            else
                disp(the_list)
            end
        end
    end
    
    %% Symbols
    methods
        function val = UpdateSymbolTable(obj, reset)
            if nargin < 2
                reset = false;
            else
                reset = logical(reset);
            end
            
            if reset
                obj.SymbolTable = struct.empty;
            end
            
            has_new_symbols = false;
            if isempty(obj.SymbolTable)
                obj.SymbolTable = mtk('symbol_table', obj.RefId);
                has_new_symbols = true;
            else                
                existing_id = uint64(length(obj.SymbolTable));
                new_symbols = mtk('symbol_table', obj.RefId, ...
                                    'from', existing_id);
                if ~isempty(new_symbols)
                    has_new_symbols = true;
                    obj.SymbolTable = [obj.SymbolTable, new_symbols];
                end                
            end
            
            % Extra processing, as new symbols added:
            if has_new_symbols
                obj.RealVarCount = max([obj.SymbolTable.basis_re]);
                if (isfield(obj.SymbolTable, 'basis_im'))
                    obj.ImaginaryVarCount = max([obj.SymbolTable.basis_im]);
                else
                    obj.ImaginaryVarCount = 0;
                end
                obj.onNewSymbolsAdded();                
            end
            
            val = obj.SymbolTable;
        end
        
        %% Destructor
        function delete(obj)
            if obj.RefId ~= 0
                try
                    mtk('release', 'matrix_system', obj.RefId);
                catch ME
                    fprintf(2, "Error deleting MatrixSystem: %s\n", ...
                        ME.message);
                end
            end
        end        
    end
    
    %% Virtual methods
    methods(Access=protected)
        function onNewSymbolsAdded(obj)
            arguments
                obj (1,1) MatrixSystem
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
            
            if nargin <= 2
            	im_name = char.empty;
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

