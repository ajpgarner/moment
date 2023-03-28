classdef SymmetrizedScenario < Abstract.Scenario
%SYMMETRIZEDSCENARIO 
%

    properties(SetAccess = private, GetAccess = public)
        BaseScenario
    end
    

    %% Construction and initialization
    methods
        function obj = SymmetrizedScenario(base)
            arguments
                base (1,1) Abstract.Scenario
            end
            
            obj.BaseScenario = base;
        end
    end
    
    %% Virtual methods
    methods(Access={?Abstract.Scenario,?MatrixSystem})
        
        function ref_id = createNewMatrixSystem(obj)
        % CREATENEWMATRIXSYSTEM Invoke mtk to create imported matrix system.
            arguments
                obj (1,1) SymmetrizedScenario
            end
            
            base_id = obj.BaseScenario.System.RefId;
                        
            ref_id = mtk('new_symmetrized_matrix_system', base_id);
        end
    end
    
  
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) SymmetrizedScenario
                mm (1,1) OpMatrix.MomentMatrix
            end            
        end
    end
end

