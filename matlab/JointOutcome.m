classdef JointOutcome < handle
    %JOINTOUTCOME Product of two or more outcomes
    
    properties(SetAccess=private, GetAccess=public)
        Constituents
        Indices
    end
    
    properties(Access={?Setting})
        real_coefs
    end
    
    methods
        function obj = JointOutcome(setting, indices)
            %JOINTOUTCOME Construct an instance of this class
             arguments
                setting (1,1) Setting
                indices (:,3) uint64 {mustBeInteger, mustBeNonnegative}
             end
            
            % Save indices that define this joint mmt outcome
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
    end
end

