classdef Outcome < handle
    %OUTCOME Measurement outcome
    properties(SetAccess={?MomentMatrix}, GetAccess=public)
        Id
    end
    
    methods
        function obj = Outcome(outcome_index)
            arguments
                outcome_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            obj.Id = outcome_index;
        end
    end
end