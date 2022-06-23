classdef MomentMatrix  < handle
    %MOMENTMATRIX A matrix of operator products. Wraps a reference to a 
    % MomentMatrix class stored within npatk.
    
    properties(Access = protected)
        symbol_matrix
        sequence_matrix
        symbol_table
        basis_real
        basis_im       
        sparse_basis_real
        sparse_basis_im
    end
    
    properties(SetAccess = protected, GetAccess = public)
        RefId = uint64(0)
        Level
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
                obj.RefId = npatk('make_moment_matrix', 'reference', ...
                    settingParams{:});
                
            elseif isa(settingParams, 'Setting')
                % Supply setting object directly
                obj.RefId = npatk('make_moment_matrix', 'reference', ...
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
                if (isempty(obj.symbol_table))
                    [obj.symbol_matrix, obj.symbol_table] = ...
                        npatk('make_moment_matrix', ...
                        'reference_id', obj.RefId, ...
                        'symbols');
                else
                    obj.symbol_matrix = npatk('make_moment_matrix', ...
                        'reference_id', obj.RefId, ...
                        'symbols');
                end
            end
            matrix = obj.symbol_matrix;
        end
        
        function matrix = SequenceMatrix(obj)
            % Defer copy of matrix until requested...
            if (isempty(obj.sequence_matrix))
                if (isempty(obj.symbol_table))
                    [obj.sequence_matrix, obj.symbol_table] = ...
                        npatk('make_moment_matrix', ...
                        'reference_id', obj.RefId, 'sequences');
                else
                    obj.sequence_matrix = npatk('make_moment_matrix', ...
                        'reference_id', obj.RefId, 'sequences');
                end
            end
            matrix = obj.sequence_matrix;
        end
        
        function table = SymbolTable(obj)
            % Defer copy of table until requested...
            if (isempty(obj.symbol_table))
                obj.symbol_table = npatk('make_moment_matrix', ...
                    'reference_id', obj.RefId, 'table');
            end
            table = obj.symbol_table;
        end
        
        function [re, im] = DenseBasis(obj)            
            if (isempty(obj.basis_real) || isempty(obj.basis_im))
                [obj.basis_real, obj.basis_im, keys] = ...
                    npatk('generate_basis', obj, 'hermitian', 'dense');
                
                obj.incorporateBasisKeys(keys);
            end
            
            re = obj.basis_real;
            im = obj.basis_im;
        end
        
        function [re, im] = SparseBasis(obj)
            if (isempty(obj.sparse_basis_real) || isempty(obj.sparse_basis_im))
                [obj.sparse_basis_real, obj.sparse_basis_im, keys] = ...
                    npatk('generate_basis', obj, 'hermitian', 'sparse');

                obj.incorporateBasisKeys(keys); 
            end
            
            re = obj.sparse_basis_real;
            im = obj.sparse_basis_im;
        end
        
    end
    
    methods(Access = private)
        function incorporateBasisKeys(obj, keys)
            % Force generation of symbol table, if not already existant
            obj.SymbolTable();
            
            % Add (or wipe) basis key fields
            z = num2cell(zeros(1,length(obj.symbol_table)));
            [obj.symbol_table.basis_re] = z{:};
            [obj.symbol_table.basis_im] = z{:};
            
            % ASSERTION: keys, and obj.symbol_table are both sorted by
            % index and contain the same indices (except 0)
            if (length(obj.symbol_table) ~= (length(keys)+1))
                error("symbol_table and basis key table do not match!");
            end
            
            % Copy keys
            for index = 2:length(obj.symbol_table)
                if (obj.symbol_table(index).symbol ~= keys(index-1, 1))
                    error("symbol_table and basis key table do not match!");
                end
                obj.symbol_table(index).basis_re = keys(index-1, 2);
                obj.symbol_table(index).basis_im = keys(index-1, 3);
            end
        end
    end
end

