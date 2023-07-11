classdef MTKMatrixSystem < handle
%MTKMATRIXSYSTEM A matrix of operator products.
% Wraps a reference to a MatrixSystem class stored within mtk.
   
    %% Properties
    properties(GetAccess = public, SetAccess = private)
        RefId = uint64(0)        
        SymbolTable = struct.empty;
        RealVarCount = uint64(0)
        ImaginaryVarCount = uint64(0);
    end
    
    %% Error messages
    properties(Constant, Access = protected)
        err_bad_scenario_type = [
            'First cell of array argument must be one of "algebraic",',...
            '"locality", "imported" or "inflation"'];
        
        err_bad_ctor_arg = ['Argument must be either a Scenario class,',...
                            ' or a cell array of parameters.'];
    end  
    
    %% Events
    events
        % Trigged when symbols are added to the matrix system.
        NewSymbolsAdded
    end
    
    %% Constructor
    methods
        function obj = MTKMatrixSystem(args)
            % Checks if args is a Scenario, or cell array, then calls
            % appropriate MEX function to initiate setting.
            % End-users are encouraged not to invoke this manually, but
            % prefer automating it through a [System-Type]Scenario class.
            
            if isa(args, 'MTKScenario')
                % Invoke Scenario's own MatrixSystem creation function
                obj.RefId = args.createNewMatrixSystem();
            elseif iscell(args)
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
                        args{1} = 'algebraic_matrix_system';
                    case "locality"
                        args{1} = 'locality_matrix_system';
                    case "imported"
                        args{1} = 'imported_matrix_system';
                    case "inflation"
                        args{1} = 'inflation_matrix_system';
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
        function varargout = List(obj, verbose)
        % LIST Gets debug information about this matrix system.
            if nargin < 2
                verbose = false;
            else
                verbose = logical(verbose);
            end
            
            if verbose
                the_list = mtk('list', 'verbose', obj.RefId);
            else
                the_list = mtk('list', obj.RefId);
            end
            
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
                reset = logical(reset(1));
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
                                
                % Trigger independent listeners
                notify(obj, 'NewSymbolsAdded');
            end
            
            val = obj.SymbolTable;
        end
    end
    
    %% Destructor
    methods        
        function delete(obj)
            if obj.RefId ~= 0
                try
                    mtk('release', 'matrix_system', obj.RefId);
                catch ME
                    fprintf(2, "Error deleting matrix system: %s\n", ...
                        ME.message);
                end
            end
        end        
    end
        
    %% CVX Methods
    methods
        function cvxVars(obj, real_name, im_name) %#ok<INUSL>
        % CVXVARS Create SDP variables associated with matrix system.
        %
        % Due to the design of CVX eschewing functional programming,
        % real_name (resp. im_name) should be set to the name desired.

            % Validate real name
            if nargin < 2
                error("Must specify real variable name.");
            else
                real_name = char(real_name);
                if ~isvarname(real_name)
                    error("Invalid real variable name '%s'", real_name);
                end
            end
            
            % Validate imaginary name
            if nargin < 3 || isempty(im_name)
                export_imaginary = false;
            	im_name = char.empty;
            else
                export_imaginary = true;
                im_name = char(im_name);
                if ~isvarname(im_name)                   
                    error("Invalid imaginary variable name '%s'", im_name);
                end
            end

            % Get handle to CVX problem            
            try
                cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
                if ~isa( cvx_problem, 'cvxprob' )
                    error( 'No CVX model exists in this scope!');
                end
            catch
                error('No CVX model exists in this scope!');
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
        function [var_A, var_B] = yalmipVars(obj)
        % YALMIPVARS Creates SDP variables associated with matrix system.
        %
        % First output corresponds to real components. 
        % Second output (optional) correponds to imaginary components.
        %
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

