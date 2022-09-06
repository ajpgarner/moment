classdef MatrixSystem < handle
    %MATRIXSYSTEM Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(SetAccess = protected, GetAccess = public)
        RefId = uint64(0)        
        SymbolTable        
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
        end
        
        function val = MakeMomentMatrix(obj, level)
            arguments
                obj (1,1) MatrixSystem
                level (1,1) {mustBeNonnegative, mustBeInteger} = 1
            end
            val = MomentMatrix(obj, level);
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
end

