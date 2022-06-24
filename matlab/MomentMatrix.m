classdef MomentMatrix  < handle
    %MOMENTMATRIX A matrix of operator products. Wraps a reference to a 
    % MomentMatrix class stored within npatk.
    
    properties(Access = protected)
        symbol_matrix
        sequence_matrix
        basis_real
        basis_im       
        sparse_basis_real
        sparse_basis_im
    end
    
    properties(SetAccess = protected, GetAccess = public)
        RefId = uint64(0)
        Dimension = uint64(0)
        Level
        SymbolTable
    end
    
    methods
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
        end
        
        function delete(obj)
            if obj.RefId ~= 0
                try
                    npatk('release', 'moment_matrix', obj.RefId);
                catch ME
                    fprintf(2, "Error while deleting MomentMatrix: %s\n", ...
                        ME.message);
                end
            end
        end
        
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
               
        function [re, im] = DenseBasis(obj)            
            if (isempty(obj.basis_real) || isempty(obj.basis_im))
                [obj.basis_real, obj.basis_im] = ...
                    npatk('generate_basis', obj, 'hermitian', 'dense');
            end
            
            re = obj.basis_real;
            im = obj.basis_im;
        end
        
        function [re, im] = SparseBasis(obj)
            if (isempty(obj.sparse_basis_real) || isempty(obj.sparse_basis_im))
                [obj.sparse_basis_real, obj.sparse_basis_im] = ...
                    npatk('generate_basis', obj, 'hermitian', 'sparse');
            end
            
            re = obj.sparse_basis_real;
            im = obj.sparse_basis_im;
        end 
    end
end

