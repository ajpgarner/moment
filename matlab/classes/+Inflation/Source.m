classdef Source < handle
    %SOURCE Classical hidden variable for causal network
    
    properties(SetAccess=private, GetAccess=public)
        Scenario
        Id
        TargetIndices
    end
    
    methods
        function obj = Source(scenario, id, targetIndices)
        % SOURCE Construct a hidden variable that affects Observables.

            % Validate parameters
            assert(nargin >= 2)
            assert(isa(scenario, 'InflationScenario'));
            assert(numel(id) == 1)
            id = uint64(id);
            if nargin < 3
                targetIndices = uint64.empty(1,0);
            else
                targetIndices = reshape(uint64(targetIndices), 1, []);
            end
            targetIndices = sort(targetIndices);
            
            % Set values
            obj.Scenario = scenario;
            obj.Id = id;
            obj.TargetIndices = targetIndices;
        end
    end
end

