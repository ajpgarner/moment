classdef Measurement < handle
    %MEASUREMENT A collection of outcomes
    
    properties(SetAccess={?MomentMatrix}, GetAccess=public)
        Id
        Name
        Outcomes
    end
    
    methods
        function obj = Measurement(mmt_index, name, num_outcomes)
            %MEASUREMENT Construct an instance of this class
            %   Detailed explanation goes here
            arguments
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                name (1,1) string
                num_outcomes (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            
            obj.Id = mmt_index;
            obj.Name = name;
            
            % Construct outcomes
            obj.Outcomes = Outcome.empty;
            for x = 1:num_outcomes
                obj.Outcomes(end+1) = Outcome(uint64(x));
            end
        end
    end
end