classdef MomentMatrix < OpMatrix.OperatorMatrix
    %MOMENTMATRIX A matrix of operator products. Wraps a reference to a
    % MomentMatrix class stored within mtk.
    
    properties(Access = {?SolvedMomentMatrix})
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
            obj = obj@OpMatrix.OperatorMatrix(matsys_input);
            
            % Save depth requested.
            obj.Level = uint64(level);
                        
            % Generate moment matrix
            [obj.Index, obj.Dimension] = mtk('moment_matrix', ...
                                               'dimension', ...
                                               obj.MatrixSystem.RefId, ...
                                               obj.Level);
            
            % Collect any new symbols generated into the environment
            obj.MatrixSystem.UpdateSymbolTable();
        end
    end
end

