classdef Outcome < handle
    %OUTCOME Measurement outcome
    properties(SetAccess={?Setting}, GetAccess=public)
        Id
        Index
    end
    
    properties(Access={?Setting})
        joint_outcomes
        real_coefs        
    end
    
    properties(Access=private)
        setting
    end

    methods
        function obj = Outcome(setting, party_index, ...
                               mmt_index, outcome_index)
            arguments
                setting (1,1) Setting
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                outcome_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            obj.setting = setting;
            
            obj.Id = outcome_index;
            obj.Index = uint64([party_index, mmt_index, outcome_index]);
           
            obj.real_coefs = zeros(0,0);
            obj.joint_outcomes = struct('indices', {}, 'outcome', {});
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
        
        function item = JointOutcome(obj, indices)
            table_index = find(arrayfun(@(s) ...
                              isequal(indices, s.indices), ...
                              obj.joint_outcomes));
            if length(table_index) ~= 1
                error("Could not find joint outcome at supplied indices.");
            end
            item = obj.joint_outcomes(table_index).outcome;
        end
    end
    
    
end