classdef Outcome < handle
    %OUTCOME Measurement outcome
    properties(SetAccess={?Setting}, GetAccess=public)
        Id
        Index
    end
    
    properties(Access={?Setting})
        real_coefs
    end

    methods
        function obj = Outcome(party_index, mmt_index, outcome_index)
            arguments
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                outcome_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            obj.Id = outcome_index;
            obj.Index = uint64([party_index, mmt_index, outcome_index]);
            obj.real_coefs = zeros(0,0);
        end
        
        function rv = Coefficients(obj)
            arguments
                obj (1,1) Outcome
            end
            % COEFFICIENTS  
            if isempty(obj.real_coefs)
                error("Outcome has not yet been associated with a " ...
                      + "moment matrix.");
            end
            
            rv = obj.real_coefs;
        end
    end
end