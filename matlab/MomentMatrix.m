classdef MomentMatrix  < handle
    %MOMENTMATRIX A matrix of operator products. Wraps a reference to a 
    % MomentMatrix class stored within npatk.
    
    properties(Access = protected)
        ref_id = 0
        symbol_matrix
        sequence_matrix
        symbol_table
    end
    
    properties(SetAccess = protected, GetAccess = public)
        Level
    end
    
    methods
        function obj = MomentMatrix(settingParams, level)
            arguments
                settingParams
                level (1,1) {mustBeNonnegative, mustBeInteger} = 1
            end
            
            % Save depth requested.
            obj.Level = level;
            
            % First, make matrix...
            % Check if settingsParams is Settings, or cell array
            %  Call npatk make_moment_matrix accordingly
            if isa(settingParams, 'cell')
                if (nargin >= 2)
                    error(["When providing a cell array input, level "...
                        "argument should be specified as part of that cell"]);
                end
                % Unpack cell into arguments
                obj.ref_id = npatk('make_moment_matrix', 'reference', ...
                    settingParams{:});
                
            elseif isa(settingParams, 'Setting')
                % Supply setting object directly
                obj.ref_id = npatk('make_moment_matrix', 'reference', ...
                    'setting', settingParams, 'level', level);
            else
                error(['First argument must be either a Setting ',...
                    'object, or a cell array of parameters.']);
            end
        end
        
        function delete(obj)
            if obj.ref_id ~= 0
                try
                    npatk('release', 'moment_matrix', obj.ref_id);
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
                        'reference_id', obj.ref_id, ...
                        'symbols');
                else
                    obj.symbol_matrix = npatk('make_moment_matrix', ...
                        'reference_id', obj.ref_id, ...
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
                        'reference_id', obj.ref_id, 'sequences');
                else
                    obj.sequence_matrix = npatk('make_moment_matrix', ...
                        'reference_id', obj.ref_id, 'sequences');
                end
            end
            matrix = obj.sequence_matrix;
        end
        
        function table = SymbolTable(obj)
            % Defer copy of table until requested...
            if (isempty(obj.symbol_table))
                obj.symbol_table = npatk('make_moment_matrix', ...
                    'reference_id', obj.ref_id, 'table');
            end
            table = obj.symbol_table;
        end
    end
end

