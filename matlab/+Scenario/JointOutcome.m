classdef JointOutcome < handle & RealObject
    %JOINTOUTCOME Product of two or more outcomes
    
    properties(SetAccess=private, GetAccess=public)
        Constituents
        Indices
    end
    
    %% Public methods
    methods
        function obj = JointOutcome(setting, indices)
            %JOINTOUTCOME Construct an instance of this class
             arguments
                setting (1,1) Scenario
                indices (:,3) uint64 {mustBeInteger, mustBeNonnegative}
             end
            
            % Superclass c'tor
            obj = obj@RealObject(setting);
            
            % Save indices that define this joint mmt outcome
            obj.Indices = indices;
            num_mmts = size(obj.Indices, 1);
            if num_mmts < 2
                error("Joint outcome must involve two or more outcomes.")
            end
            
            % Copy refs to outcomes that make up this joint mmt outcome
            obj.Constituents = Scenario.Outcome.empty([1, 0]);
            for row = 1:num_mmts
                obj.Constituents(end+1) = setting.get(obj.Indices(row, :));
            end
        end
        
        function joint_item = mtimes(objA, objB)
            arguments
                objA (1,1)
                objB (1,1)
            end
                        
            % Should only occur when A is a built-in object
            if ~isa(objA, 'Scenario.JointOutcome')
                joint_item = mtimes@RealObject(objA, objB);
                return
            end
            
            if isa(objB, 'Scenario.JointOutcome')                
                if objA.Scenario ~= objB.Scenario
                    error("Can only combine objects from the same setting.");
                end
                if ~isempty(intersect(objA.Indices(:,1), ...
                                      objB.Indices(:,1)))
                    error("_*_ can only be used to form joint "...
                          + "probability outcomes (i.e. all outcomes "...
                          + "must be from different parties).");
                end
                indices = sortrows(vertcat(objA.Indices, objB.Indices));
                joint_item = objA.Scenario.get(indices);
            elseif isa(objB, 'Scenario.Outcome')                
                if objA.Scenario ~= objB.Scenario
                    error("Can only combine objects from the same setting.");
                end
                if ismember(objB.Index(1), objA.Indices(:,1))
                    error("_*_ can only be used to form joint "...
                          + "probability outcomes (i.e. all outcomes "...
                          + "must be from different parties).");
                end
                indices = sortrows(vertcat(objA.Indices, objB.Index));
                joint_item = objA.Scenario.get(indices);
            else
                % Fall back to superclass:~
                joint_item = mtimes@RealObject(objA, objB);
            end
        end
    end  
end