classdef Setting < handle
    %SETTING A scenario involving multiple agents with measurements.
    %   
      
    properties(GetAccess = public, SetAccess = protected)
        Parties
        HasMomentMatrix
    end
    
    properties(Access = private)
        moment_matrix
    end
    
    methods
        function obj = Setting(initial_parties)
            arguments
                initial_parties (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            obj.Parties = Party.empty;
            if (initial_parties >=1 )
                for x = 1:initial_parties 
                    obj.Parties(end+1) = Party(obj, x);
                end
            end
        end
        
        function AddParty(obj, name)
            arguments
                obj (1,1) Setting
                name (1,1) string
            end
            
            next_id = length(obj.Parties)+1;
            if nargin >=2
                obj.Parties(end+1) = Party(obj, next_id, name);
            else
                obj.Parties(end+1) = Party(obj, next_id);
            end
            
            obj.Parties(end).setting = obj;
        end
        
        function mm_out = MakeMomentMatrix(obj, depth, skip_bind)
            arguments
                obj (1,1) Setting 
                depth (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                skip_bind (1,1) logical = false
            end
            obj.moment_matrix = MomentMatrix(obj, depth);          
            if ~skip_bind
                obj.do_bind(obj.moment_matrix)
            end
            mm_out = obj.moment_matrix;
        end
        
        function val = get.HasMomentMatrix(obj)
            val = ~isempty(obj.moment_matrix);
        end
        
        function item = get(obj, index)
            arguments
                obj (1,1) Setting
                index (:,:) uint64
            end
            get_what = size(index, 2);
            get_joint = size(index, 1) > 1;
            
            if get_joint
                % Joint outcome object
                if get_what ~= 3
                    error("Multiple indices must be in form" ...
                           + " [[partyA, mmtA, outcomeA]; ... ].");
                end
                index = sortrows(index);
                leading_item = obj.Parties(index(1, 1)).Measurements(index(1, 2)).Outcomes(index(1, 3));
                item = leading_item.JointOutcome(index);
            else            
                % Otherwise, single object [party, mmt, or outcome]
                switch get_what
                    case 1
                        item = obj.Parties(index(1));
                    case 2
                        item = obj.Parties(index(1)).Measurements(index(2));
                    case 3
                        item = obj.Parties(index(1)).Measurements(index(2)).Outcomes(index(3));
                    otherwise
                        error("Index should be [party], [party, mmt], or [party, mmt, outcome].");
                end
            end
        end
    end
    
    methods(Access=private)
        function do_bind(obj, mm)
             arguments
                obj (1,1) Setting
                mm (1,1) MomentMatrix
             end
             p_table = mm.ProbabilityTable;
             for p_row = p_table
                 seq_len = size(p_row.indices, 1);
                 if seq_len == 0
                     continue
                 end
                 leading_outcome = obj.get(p_row.indices(1,:));
                 
                 if seq_len == 1
                     % Directly link co-efficients with outcome
                     leading_outcome.setCoefficients(p_row.real_coefficients);
                 else
                     % Register co-effs as joint outcome
                     joint_outcome = JointOutcome(obj, p_row.indices);
                     joint_outcome.setCoefficients(p_row.real_coefficients);
                     
                     leading_outcome.joint_outcomes(end+1).indices = ...
                         p_row.indices;
                     leading_outcome.joint_outcomes(end).outcome = ...
                        joint_outcome;
                 end
             end
        end
    end
end

