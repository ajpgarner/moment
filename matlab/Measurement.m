classdef Measurement < handle
    %MEASUREMENT A collection of outcomes
    
    properties(SetAccess=private, GetAccess=public)
        Id
        Index
        Name
        Outcomes
    end
    
    methods
        function obj = Measurement(party_index, mmt_index, name, num_outcomes)
            %MEASUREMENT Construct an instance of this class
            arguments
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                name (1,1) string
                num_outcomes (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            
            obj.Id = mmt_index;
            obj.Index = uint64([party_index, mmt_index]);
            obj.Name = name;
            
            % Construct outcomes
            obj.Outcomes = Outcome.empty;
            for x = 1:num_outcomes
                obj.Outcomes(end+1) = Outcome(obj.Index(1), ...
                                              obj.Index(2), uint64(x));
            end
        end
    end
end