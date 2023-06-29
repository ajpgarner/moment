classdef Outcome
    %OUTCOME Measurement outcome
    properties(SetAccess={?LocalityScenario}, GetAccess=public)
        Scenario
        Id
        Index
        Value
    end
        
    methods
        function obj = Outcome(setting, party_index, ...
                               mmt_index, outcome_index, ...
                               value)
            arguments
                setting (1,1) LocalityScenario
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                outcome_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                value (1,1) double = 0
            end
            
            obj.Scenario = setting;
            obj.Id = outcome_index;
            obj.Index = uint64([party_index, mmt_index, outcome_index]);
            obj.Value = value;
           
            %obj.joint_outcomes = struct('indices', {}, 'outcome', {});
        end
        
        function joint_item = mtimes(objA, objB)
            arguments
                objA (1,1)
                objB (1,1)
            end
            
            % Should only occur when A is a built-in object
            if ~isa(objA, 'Locality.Outcome')
                joint_item = mtimes@RealObject(objA, objB);
                return
            end
            
            if isa(objB, 'Locality.JointOutcome')
                if objA.Scenario ~= objB.Scenario
                    error("Can only combine objects from the same setting.");
                end
                if ismember(objA.Index(1), objB.Indices(:,1))
                    error("_*_ can only be used to form joint "...
                          + "probability outcomes (i.e. outcomes must "...
                          + "be from different parties).");
                end
                
                indices = sortrows(vertcat(objA.Index, objB.Indices));
                joint_item = objA.JointOutcome(indices);
            elseif isa(objB, 'Locality.Outcome')
                if objA.Scenario ~= objB.Scenario
                    error("Can only combine objects from the same setting.");
                end
                if objA.Index(1) == objB.Index(1)
                    error("_*_ can only be used to form joint "...
                          + "probability outcomes (i.e. outcomes must "...
                          + "be from different parties).");
                end
                
                indices = sortrows(vertcat(objA.Index, objB.Index));
                joint_item = objA.JointOutcome(indices);
            else
                % Fall back to superclass:~
                joint_item = mtimes@RealObject(objA, objB);                
            end         
        end
   
        function item = JointOutcome(obj, indices)
            arguments
                obj (1,1) Locality.Outcome
                indices (:,:) uint64
            end
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