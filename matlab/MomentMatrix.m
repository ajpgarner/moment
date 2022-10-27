classdef MomentMatrix < OperatorMatrix
    %MOMENTMATRIX A matrix of operator products. Wraps a reference to a
    % MomentMatrix class stored within npatk.
    
    properties(Access = {?SolvedMomentMatrix})
        mono_basis_real
        mono_basis_im
        sparse_mono_basis_real
        sparse_mono_basis_im
        basis_real
        basis_im
        sparse_basis_real
        sparse_basis_im
        probability_table
    end
    
    properties(SetAccess = protected, GetAccess = public)
        Level = uint64(0)
    end
    
    methods
        %% Constructor
        function obj = MomentMatrix(settingParams, level)
            arguments
                settingParams
                level (1,1) {mustBeNonnegative, mustBeInteger} = 1
            end
            
            % First, get or make scenario, or take in supplied MatrixSystem            
            if isa(settingParams, 'cell') || isa(settingParams, 'Scenario')
                matsys_input = MatrixSystem(settingParams);
            elseif isa(settingParams, 'MatrixSystem')
                matsys_input = settingParams;
            else
                error(['First argument must be either a Scenario ',...
                       'object, a cell array of parameters, or a ', ...
                       'MatrixSystem.']);
            end
            
            % Register matrix system            
            obj = obj@OperatorMatrix(matsys_input);
            
            % Save depth requested.
            obj.Level = uint64(level);
                        
            % Generate moment matrix
            [obj.Index, obj.Dimension] = npatk('moment_matrix', ...
                                               'dimension', ...
                                               obj.MatrixSystem.RefId, ...
                                               obj.Level);
            
            % Collect any new symbols generated into the environment
            obj.MatrixSystem.UpdateSymbolTable();
                    
        end
                        
        %% Accessors for probability table
        function p_table = ProbabilityTable(obj)
            % PROBABILITYTABLE A struct-array indicating how each
            %   measurement outcome can be expressed in terms of real
            %   basis elements (including implied probabilities that do not
            %   directly exist as operators in the moment matrix).
            if (isempty(obj.probability_table))
                obj.probability_table = npatk('probability_table', ...
                                              obj.MatrixSystem.RefId);
            end
            p_table = obj.probability_table;
        end
        
        function result = MeasurementCoefs(obj, indices)
            arguments
                obj (1,1) MomentMatrix
                indices (:,2) uint64
            end            
            parties = indices(:, 1);
            if length(parties) ~= length(unique(parties))
                error("Measurements must be from different parties.");
            end
            
            result = npatk('probability_table', ...
                           obj.MatrixSystem.RefId, indices);
        end
        
              
        %% Accessors for basis in various forms
        function [re, im] = DenseBasis(obj)
            % DENSEBASIS Get the basis as a cell array of dense matrices.
            if (isempty(obj.basis_real) || isempty(obj.basis_im))
                [obj.basis_real, obj.basis_im] = ...
                    npatk('generate_basis', ...
                          obj.MatrixSystem.RefId, obj.Index, ...
                          'hermitian', 'dense');
            end
            
            re = obj.basis_real;
            im = obj.basis_im;
        end
        
        function [re, im] = SparseBasis(obj)
            % SPARSEBASIS Get the basis as a cell array of sparse matrices.
            if (isempty(obj.sparse_basis_real) ...
                    || isempty(obj.sparse_basis_im))
                [obj.sparse_basis_real, obj.sparse_basis_im] = ...
                    npatk('generate_basis', ...
                          obj.MatrixSystem.RefId, obj.Index, ...
                          'hermitian', 'sparse');
            end
            
            re = obj.sparse_basis_real;
            im = obj.sparse_basis_im;
        end
        
        function [re, im] = MonolithicBasis(obj, sparse)
            % MONOLITHICBASIS Return the basis as a pair of partially-
            %   flattened matrices, such that each row represents one basis
            %   element, with length of Dimension*Dimension.
            arguments
                obj (1,1) MomentMatrix
                sparse (1,1) logical = true
            end
            
            if sparse
                [re, im] = obj.SparseMonolithicBasis();
            else
                [re, im] = obj.DenseMonolithicBasis();
            end
        end
        
        function [re, im] = SparseMonolithicBasis(obj)
            arguments
                obj (1,1) MomentMatrix
            end
            % TODO: Check length
            
            if (isempty(obj.sparse_mono_basis_real) || ...
                isempty(obj.sparse_mono_basis_im))
                    [obj.sparse_mono_basis_real, ...
                     obj.sparse_mono_basis_im] = ...
                        npatk('generate_basis', ...
                              obj.MatrixSystem.RefId, obj.Index, ...
                              'monolith', 'hermitian', 'sparse');
            end            
            re = obj.sparse_mono_basis_real;
            im = obj.sparse_mono_basis_im;
        end
     
        function [re, im] = DenseMonolithicBasis(obj)
            arguments
                obj (1,1) MomentMatrix
            end
            % TODO: Check length
            
            if (isempty(obj.mono_basis_real) || ...
                isempty(obj.mono_basis_im))
                    [obj.mono_basis_real, ...
                     obj.mono_basis_im] = ...
                        npatk('generate_basis', ...
                              obj.MatrixSystem.RefId, obj.Index, ...
                              'monolith', 'hermitian', 'dense');
            end            
            re = obj.sparse_mono_basis_real;
            im = obj.sparse_mono_basis_im;
        end   
        
    end
    
    %% Virtual method overloads
    methods(Access=protected)       
        function matrix = queryForSymbolMatrix(obj)
            % Defer copy of matrix until requested...
            matrix = npatk('moment_matrix', ...
                            obj.MatrixSystem.RefId, obj.Level, ...
                            'symbols');           
        end
        
        function matrix = queryForSequenceMatrix(obj)
            % Defer copy of matrix until requested...
            matrix = npatk('moment_matrix', ...
                           obj.MatrixSystem.RefId, obj.Level, ...
                           'sequences');           
        end
    end
    
    %% CVX Methods
    methods
        function cvxVars(obj, re_name, im_name)
            arguments
                obj (1,1) MomentMatrix
                re_name (1,:) char {mustBeValidVariableName}
                im_name (1,:) char = char.empty
            end
            
            % Check if exporting real, or real & imaginary
            export_imaginary = ~isempty(im_name);
            if export_imaginary
             	if ~isvarname(im_name)
             		error("Invalid variable name.");
             	end
            end
            
            % Propagate CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            % Export
            if ~export_imaginary
                obj.MatrixSystem.cvxCreateVars(re_name)
                assignin('caller', re_name, eval(re_name));
            else
                obj.MatrixSystem.cvxCreateVars(re_name, im_name)
                assignin('caller', re_name, eval(re_name));
                assignin('caller', im_name, eval(im_name));
            end
        end
            
        function out_M = cvxHermitianBasis(obj, a, b)
            % Get handle to CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            % Multiple variables by basis to make matrix
            [real_basis, im_basis] = obj.MonolithicBasis(true);
            
            expression M(obj.Dimension, obj.Dimension)
            M(:,:) = reshape(transpose(a) * real_basis ...
                + transpose(b) * im_basis, ...
                [obj.Dimension, obj.Dimension]);
                        
            % Output handles to cvx objects
            out_M = M;
        end
        
        function out_M = cvxSymmetricBasis(obj, a)
            % Get handle to CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            % Multiple variables by basis to make matrix
            [real_basis, ~] = obj.MonolithicBasis(true);
            expression M(obj.Dimension, obj.Dimension);
            M(:,:) = reshape(transpose(a) * real_basis, ...
                [obj.Dimension, obj.Dimension]);
            
            % Output handles to cvx objects
            out_M = M;
        end
    end
    
    %% Yalmip Methods
    methods
        function varargout = yalmipVars(obj)
            % Forward variable creation to matrix system.
            
            if nargout == 1
                export_imaginary = false;
            elseif nargout == 2
                export_imaginary = true;
            else
                error("One or two outputs expected.");
            end
            
            varargout = cell(1, nargout);
            if ~export_imaginary                
                varargout{1} = obj.MatrixSystem.yalmipCreateVars();
            else
                [varargout{1}, varargout{2}] = ...
                   obj.MatrixSystem.yalmipCreateVars();
            end
        end
        
        function out_M = yalmipHermitianBasis(obj, a, b)
 
            % Multiple variables by basis to make matrix
            [real_basis, im_basis] = obj.MonolithicBasis(true);
                       
            % Matrix
            out_M = reshape(transpose(a) * real_basis ...
                + transpose(b) * im_basis, ...
                [obj.Dimension, obj.Dimension]);
        end
        
         function out_M = yalmipSymmetricBasis(obj, a)
            % Multiple variables by basis to make matrix
            [real_basis, ~] = obj.MonolithicBasis(true);
           
            % Matrix
            out_M = reshape(transpose(a) * real_basis, ...
                [obj.Dimension, obj.Dimension]);
        end
    end
end

