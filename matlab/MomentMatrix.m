classdef MomentMatrix  < handle
    %MOMENTMATRIX A matrix of operator products. Wraps a reference to a
    % MomentMatrix class stored within npatk.
    
    properties(Access = {?SolvedMomentMatrix})
        symbol_matrix
        sequence_matrix
        mono_basis_real
        mono_basis_im
        basis_real
        basis_im
        sparse_basis_real
        sparse_basis_im
        probability_table
    end
    
    properties(SetAccess = protected, GetAccess = public)
        RefId = uint64(0)
        Dimension = uint64(0)
        Level
        SymbolTable
        RealBasisSize = uint64(0)
        ImaginaryBasisSize = uint64(0)
    end
    
    methods
        %% Constructor
        function obj = MomentMatrix(settingParams, level)
            arguments
                settingParams
                level (1,1) {mustBeNonnegative, mustBeInteger} = 1
            end
            
            % Save depth requested.
            obj.Level = uint64(level);
            
            % First, make matrix...
            % Check if settingsParams is Settings, or cell array
            %  Call npatk make_moment_matrix accordingly
            if isa(settingParams, 'cell')
                if (nargin >= 2)
                    error(["When providing a cell array input, level "...
                        "argument should be specified as part of that cell"]);
                end
                % Unpack cell into arguments
                [obj.RefId, obj.SymbolTable, obj.Dimension] = ...
                    npatk('make_moment_matrix', ...
                    'reference', settingParams{:});
                
            elseif isa(settingParams, 'Setting')
                % Supply setting object directly
                [obj.RefId, obj.SymbolTable, obj.Dimension] = ...
                    npatk('make_moment_matrix', 'reference', ...
                    'setting', settingParams, 'level', level);
            else
                error(['First argument must be either a Setting ',...
                    'object, or a cell array of parameters.']);
            end
            
            % Count NNZ for basis sizes
            obj.RealBasisSize = nnz([obj.SymbolTable.basis_re]);
            obj.ImaginaryBasisSize = nnz([obj.SymbolTable.basis_im]);
            
        end
        
        %% Destructor
        function delete(obj)
            if obj.RefId ~= 0
                try
                    npatk('release', 'moment_matrix', obj.RefId);
                catch ME
                    fprintf(2, "Error deleting MomentMatrix: %s\n", ...
                        ME.message);
                end
            end
        end
        
        %% Accessors for matrix representations
        function matrix = SymbolMatrix(obj)
            % Defer copy of matrix until requested...
            if (isempty(obj.symbol_matrix))
                obj.symbol_matrix = npatk('make_moment_matrix', ...
                    'reference_id', obj.RefId, ...
                    'symbols');
            end
            matrix = obj.symbol_matrix;
        end
        
        function matrix = SequenceMatrix(obj)
            % Defer copy of matrix until requested...
            if (isempty(obj.sequence_matrix))
                obj.sequence_matrix = npatk('make_moment_matrix', ...
                    'reference_id', obj.RefId, 'sequences');
            end
            matrix = obj.sequence_matrix;
        end
        
        %% Accessors for probability table
        function p_table = ProbabilityTable(obj)
            % PROBABILITYTABLE A struct-array indicating how each
            %   measurement outcome can be expressed in terms of real
            %   basis elements (including implied probabilities that do not
            %   directly exist as operators in the moment matrix).
            if (isempty(obj.probability_table))
                obj.probability_table = npatk('probability_table', obj);
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
            
            result = npatk('probability_table', obj, indices);
        end
        
        
        
        %% Accessors for basis in various forms
        function [re, im] = DenseBasis(obj)
            % DENSEBASIS Get the basis as a cell array of dense matrices.
            if (isempty(obj.basis_real) || isempty(obj.basis_im))
                [obj.basis_real, obj.basis_im] = ...
                    npatk('generate_basis', obj, 'hermitian', 'dense');
            end
            
            re = obj.basis_real;
            im = obj.basis_im;
        end
        
        function [re, im] = SparseBasis(obj)
            % SPARSEBASIS Get the basis as a cell array of sparse matrices.
            if (isempty(obj.sparse_basis_real) ...
                    || isempty(obj.sparse_basis_im))
                [obj.sparse_basis_real, obj.sparse_basis_im] = ...
                    npatk('generate_basis', obj, 'hermitian', 'sparse');
            end
            
            re = obj.sparse_basis_real;
            im = obj.sparse_basis_im;
        end
        
        function [re, im] = MonolithicBasis(obj, sparse)
            % MONOLITHICBASIS Return the basis as a pair of partially-
            %   flattened matrices, such that each row represents one basis
            %   element, with length of Dimension*Dimension.
            arguments
                obj
                sparse (1,1) logical = true
            end
            if (isempty(obj.mono_basis_real) || isempty(obj.mono_basis_im))
                if sparse
                    sod = 'sparse';
                else
                    sod = 'dense';
                end
                [obj.mono_basis_real, obj.mono_basis_im] = ...
                    npatk('generate_basis', obj, ...
                    'monolith', 'hermitian', sod);
            end
            
            re = obj.mono_basis_real;
            im = obj.mono_basis_im;
        end
    end
    
    %% CVX Methods
    methods
        function [out_a, out_b, out_M] = cvxHermitianBasis(obj)
            % Get handle to CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            % Multiple variables by basis to make matrix
            [real_basis, im_basis] = obj.MonolithicBasis(false);
            variable a(obj.RealBasisSize);
            variable b(obj.ImaginaryBasisSize);
            expression M(obj.Dimension, obj.Dimension)
            M(:,:) = reshape(transpose(a) * real_basis ...
                + transpose(b) * im_basis, ...
                [obj.Dimension, obj.Dimension]);
            
            % Output handles to cvx objects
            out_a = a;
            out_b = b;
            out_M = M;
        end
        
        function [out_a, out_M] = cvxSymmetricBasis(obj)
            % Get handle to CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            % Multiple variables by basis to make matrix
            [real_basis, ~] = obj.MonolithicBasis(false);
            variable a(obj.RealBasisSize);
            expression M(obj.Dimension, obj.Dimension);
            M(:,:) = reshape(transpose(a) * real_basis, ...
                [obj.Dimension, obj.Dimension]);
            
            % Output handles to cvx objects
            out_a = a;
            out_M = M;
        end
    end
end

