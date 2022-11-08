classdef OperatorMatrix < handle
    %OPERATORMATRIX Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(SetAccess = protected, GetAccess = public)
        MatrixSystem
        Index = uint64.empty
        Dimension = uint64.empty
        
        SymbolMatrix
        SequenceMatrix
        
        RealSymbols
        ImaginarySymbols
        RealSymbolMask
        ImaginarySymbolMask
    end
    
    properties(Access = protected)
        symbol_matrix
        sequence_matrix
        mono_basis_real
        mono_basis_im
        sparse_mono_basis_real
        sparse_mono_basis_im
        basis_real
        basis_im
        sparse_basis_real
        sparse_basis_im
    end
    
    methods
        function obj = OperatorMatrix(matrixSystem)
            %OPERATORMATRIX Construct an instance of this class
            arguments
                matrixSystem (1,1) MatrixSystem
            end
            
            obj.MatrixSystem = matrixSystem;
            obj.symbol_matrix = [];
            obj.sequence_matrix = [];
        end
        
        
        %% Accessors for matrices
        function val = get.SymbolMatrix(obj)
            if isempty(obj.symbol_matrix)
                if (isempty(obj.Index))
                    error("No index was associated with operator matrix.");
                end
                
                obj.symbol_matrix = npatk('operator_matrix', 'symbols', ...
                    obj.MatrixSystem.RefId, ...
                    obj.Index);
            end
            val = obj.symbol_matrix;
        end
        
        function val = get.SequenceMatrix(obj)
            if isempty(obj.sequence_matrix)
                if (isempty(obj.Index))
                    error("No index was associated with operator matrix.");
                end
                
                obj.sequence_matrix = npatk('operator_matrix', ...
                    'sequences', ...
                    obj.MatrixSystem.RefId, ...
                    obj.Index);
            end
            val = obj.sequence_matrix;
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
                obj (1,1) OperatorMatrix
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
                obj (1,1) OperatorMatrix
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
                obj (1,1) OperatorMatrix
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
        
    %% CVX Methods
    methods
        function cvxVars(obj, re_name, im_name)
            arguments
                obj (1,1) OperatorMatrix
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

