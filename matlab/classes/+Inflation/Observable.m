classdef Observable < handle
    %MEASUREMENT A classical measurement for causal network scenarios
    
    properties(SetAccess=private, GetAccess=public)
        Setting
        Id
        OutcomeCount
    end
    
    properties(GetAccess=public, SetAccess={?Inflation.Observable,?InflationScenario})
        OperatorOffset = 0;
    end
    
    methods
        function obj = Observable(setting, id, outcomes)
            arguments
                setting (1,1) InflationScenario
                id (1,1) uint64
                outcomes (1,1) uint64
            end
            obj.Setting = setting;
            obj.Id = id;
            obj.OutcomeCount = outcomes;
        end
    end
end

