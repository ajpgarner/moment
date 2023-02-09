classdef Source < handle
    %SOURCE Classical hidden variable for causal network
    
    properties(SetAccess=private, GetAccess=public)
        Setting
        Id
        TargetIndices
    end
    
    methods
        function obj = Source(setting, id, targetIndices)
            arguments
                setting (1,1) InflationScenario
                id (1,1) uint64
                targetIndices (1,:) uint64
            end
            obj.Setting = setting;
            obj.Id = id;
            obj.TargetIndices = targetIndices;
        end
    end
end

