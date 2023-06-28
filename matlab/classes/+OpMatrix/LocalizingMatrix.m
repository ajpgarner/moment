classdef LocalizingMatrix < OpMatrix.OperatorMatrix
    %LocalizingMatrix A matrix of operator products. Wraps a reference to a
    % LocalizingMatrix class stored within mtk.
    
    properties(SetAccess = protected, GetAccess = public)
        Level = uint64(0)
        Operators = uint64.empty(1,0)
    end
    
    methods
        %% Constructor
        function obj = LocalizingMatrix(setting, operators, level)
            arguments
                setting (1,1) MTKScenario
                operators
                level (1,1) {mustBeNonnegative, mustBeInteger} = 1
            end
            
            % Register matrix system            
            obj = obj@OpMatrix.OperatorMatrix(setting.System);
            
            % Save depth requested.
            obj.Level = uint64(level);
            
            % Cast operator string:~
            if ~isnumeric(operators) 
                error("Operators must be provided as an array of indices.");
            end
            obj.Operators = uint64(reshape(operators, 1,[]));
            
            % Generate localizing matrix
            [obj.Index, obj.Dimension] = mtk('localizing_matrix', ...
                                               'matlab_indexing', ...
                                               'dimension', ...
                                               obj.MatrixSystem.RefId, ...
                                               obj.Level, obj.Operators);
            
            % Collect any new symbols generated into the environment
            obj.MatrixSystem.UpdateSymbolTable();
        end
    end
end

