classdef AlgebraicScenario < Scenario
    %ALGEBRAICSCENARIO
    
    properties(GetAccess = public, SetAccess = protected)
        OperatorCount
    end
    
    %% Constructor
    methods
        function obj = AlgebraicScenario(op_count, rules)           
            
            % Superclass c'tor
            obj = obj@Scenario();
            
            obj.OperatorCount = op_count;
        end 
    end
    
    %% Friend/interface methods
    methods(Access={?Scenario,?MatrixSystem})
        % Query for a matrix system
        function ref_id = createNewMatrixSystem(obj)
            arguments
                obj (1,1) AlgebraicScenario
            end
            ref_id = npatk('new_algebraic_matrix_system', ...
                           obj.OperatorCount);
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) AlgebraicScenario
                mm (1,1) MomentMatrix
            end
        end
    end
end

