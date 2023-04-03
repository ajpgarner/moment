classdef SymmetrizedScenario < Abstract.Scenario
%SYMMETRIZEDSCENARIO 
%

    properties(SetAccess = private, GetAccess = public)
        BaseScenario
        Group = Symmetry.Group.empty(0,0);
    end
    
    %% Construction and initialization
    methods
        function obj = SymmetrizedScenario(base, generators)
        % SYMMETRIZEDSCENARIO Constructs a symmetrized version of a scenario.
        %
        % PARAMS:
        %   base - The base scenario
        %   generators - The generators of the symmetry group, as acting on
        %                the fundamental operators of the scenario.
        %
            arguments
                base (1,1) Abstract.Scenario
                generators
            end
            
            obj.BaseScenario = base;            
            obj.Group = Symmetry.Group(obj, generators);
            
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
                        
            ref_id = mtk('new_symmetrized_matrix_system', ...
                         base_id, obj.Group.Generators);
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

