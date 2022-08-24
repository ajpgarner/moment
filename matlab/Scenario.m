classdef Scenario < handle
    %SETTING A scenario involving multiple agents with measurements.
    %   
      
    properties(GetAccess = public, SetAccess = protected)
        Parties
        MeasurementsPerParty
        HasMomentMatrix
        Normalization
    end
    
    properties(Access = private)
        moment_matrix
    end
    
    methods
        function obj = Scenario(initial_parties)
            arguments
                initial_parties (1,1) uint64 {mustBeInteger, mustBeNonnegative}
            end
            
            obj.Normalization = Scenario.Normalization(obj);
            
            obj.Parties = Scenario.Party.empty;
            if (initial_parties >=1 )
                for x = 1:initial_parties 
                    obj.Parties(end+1) = Scenario.Party(obj, x);
                end
            end
        end
        
        function AddParty(obj, name)
            arguments
                obj (1,1) Scenario
                name (1,1) string = ''
            end
            import Scenario.Party
            
            next_id = length(obj.Parties)+1;
            if nargin >=2
                obj.Parties(end+1) = Party(obj, next_id, name);
            else
                obj.Parties(end+1) = Party(obj, next_id);
            end
            obj.invalidateMomentMatrix();
        end
        
        function val = get.MeasurementsPerParty(obj)
            val = zeros(1, length(obj.Parties));
            for party_id = 1:length(obj.Parties)
                val(party_id) = length(obj.Parties(party_id).Measurements);
            end
        end
        
        function mm_out = MakeMomentMatrix(obj, depth, skip_bind)
            arguments
                obj (1,1) Scenario 
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
                obj (1,1) Scenario
                index (:,:) uint64
            end
            get_what = size(index, 2);
            get_joint = size(index, 1) > 1;
            
            if isempty(index)
                item = obj.Normalization;
                return;
            end
            
            if get_joint
                % Joint outcome object
                if get_what == 2
                    index = sortrows(index);
                    leading_mmt = obj.Parties(index(1, 1)).Measurements(index(1, 2));
                    item = leading_mmt.JointMeasurement(index);
                elseif get_what == 3
                    index = sortrows(index);
                    leading_item = obj.Parties(index(1, 1)).Measurements(index(1, 2)).Outcomes(index(1, 3));
                    item = leading_item.JointOutcome(index); 
                else
                    error("Multiple indices must be in form" ...
                           + " [[partyA, mmtA, outcomeA]; ... ].");
                end
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
        
        function val = FCTensor(obj, tensor)
            arguments
                obj (1,1) Scenario
                tensor double
            end
            if isempty(obj.moment_matrix)
                error("Cannot apply full-correlator tensor before " ... 
                      + "MomentMatrix has been generated.");
            end
            fc = Scenario.FullCorrelator(obj);
            val = fc.linfunc(tensor);
        end
        
        function val = FCIndex(obj, index)
            arguments
                obj (1,1) Scenario
                index (1,:) uint64
            end
            if isempty(obj.moment_matrix)
                error("Cannot apply full-correlator tensor before " ... 
                      + "MomentMatrix has been generated.");
            end
            fc = Scenario.FullCorrelator(obj);
            val = fc.at(index);
        end
    end
    
    methods(Access={?Scenario,?Scenario.Party})
        function invalidateMomentMatrix(obj)
            obj.moment_matrix = MomentMatrix.empty;
        end
        
        function make_joint_mmts(obj, party_id, new_mmt)
            arguments
                obj (1,1) Scenario
                party_id (1,1) uint64
                new_mmt (1,1) Scenario.Measurement
            end
            % First, add new measurement to lower index parties
            if party_id > 1
                for i=1:(party_id-1)
                    for m = obj.Parties(i).Measurements
                        m.addJointMmt(new_mmt);
                    end
                end
            end
            
            % Second, make joint measurement with higher index parties
            for i=(party_id+1):length(obj.Parties)
                for m = obj.Parties(i).Measurements
                    new_mmt.addJointMmt(m);
                end
            end
        end
    end
    
   
    
    methods(Access=private)
        function do_bind(obj, mm)
             arguments
                obj (1,1) Scenario
                mm (1,1) MomentMatrix
             end
             p_table = mm.ProbabilityTable;
             for p_row = p_table
                 seq_len = size(p_row.indices, 1);
                 
                 % Special case 0 and 1
                 if seq_len == 0
                     if p_row.sequence == "1"
                         obj.Normalization.setCoefficients(...
                                p_row.real_coefficients);
                     end
                     continue;
                 end
                 
                 leading_outcome = obj.get(p_row.indices(1,:));
                 
                 if seq_len == 1
                     % Directly link co-efficients with outcome
                     leading_outcome.setCoefficients(p_row.real_coefficients);
                 else
                     % Register co-effs as joint outcome
                     joint_outcome = Scenario.JointOutcome(obj, p_row.indices);
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

