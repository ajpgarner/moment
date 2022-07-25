classdef JointOutcome < handle
    %JOINTOUTCOME Product of two or more outcomes
    
    properties(SetAccess=private, GetAccess=public)
        Constituents
        Indices
    end
    
    properties(Access={?Setting})
        real_coefs
        setting
    end
    
    methods
        function obj = JointOutcome(setting, indices)
            %JOINTOUTCOME Construct an instance of this class
             arguments
                setting (1,1) Setting
                indices (:,3) uint64 {mustBeInteger, mustBeNonnegative}
             end
            
            % Save indices that define this joint mmt outcome
            obj.setting = setting;
            obj.Indices = indices;
            num_mmts = size(obj.Indices, 1);
            if num_mmts < 2
                error("Joint outcome must involve two or more outcomes.")
            end
            
            % Copy refs to outcomes that make up this joint mmt outcome
            obj.Constituents = Outcome.empty([1, 0]);
            for row = 1:num_mmts
                obj.Constituents(end+1) = setting.get(obj.Indices(row, :));
            end
        end
        
        function rv = Coefficients(obj)
            arguments
                obj (1,1) JointOutcome
            end
            if isempty(obj.real_coefs)
                error("JointOutcome was not associated with a " ...
                      + "moment matrix.");
            end
            
            rv = obj.real_coefs;
        end
        
        function joint_item = mtimes(objA, objB)
            arguments
                objA (1,1) JointOutcome
                objB (1,1) {mustBeOutcomeOrJointOutcome}
            end
            
            if isa(objB, 'JointOutcome')
                if ~isempty(intersect(objA.Indices(:,1), ...
                                      objB.Indices(:,1)))
                    error("_*_ can only be used to form joint "...
                          + "probability outcomes (i.e. all outcomes "...
                          + "must be from different parties).");
                end
                
                % TODO: Multiparty check
                indices = sortrows(vertcat(objA.Indices, objB.Indices));
            else
                if ismember(objB.Index(1), objA.Indices(:,1))
                    error("_*_ can only be used to form joint "...
                          + "probability outcomes (i.e. all outcomes "...
                          + "must be from different parties).");
                end
                indices = sortrows(vertcat(objA.Indices, objB.Index));
            end            
            joint_item = objA.setting.get(indices);
        end
    end
end

%% Private functions
function mustBeOutcomeOrJointOutcome(a)
    if ~(isa(a, 'Outcome') || isa(a, 'JointOutcome'))
        eid = 'mustBeOutcomeOrJointOutcome:isNot';
        emsg = 'Input must be Outcome or JointOutcome.';
        throwAsCaller(MException(eid, emsg))
    end
end
