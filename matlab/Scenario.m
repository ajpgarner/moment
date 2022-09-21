classdef Scenario < handle
    %SETTING A scenario involving multiple agents with measurements.
    %
    
    properties(GetAccess = public, SetAccess = protected)
        Parties
        Normalization
        MeasurementsPerParty
        OperatorsPerParty
        HasMatrixSystem
    end
    
    properties(Access = private)
        matrix_system
    end
    
    properties(Constant, Access = private)
        err_locked = ['This Scenario object is locked, and no further changes are possible. ', ...
            'This is because it has been associated with a MatrixSystem ', ...
            '(e.g. at least one MomentMatrix has already been generated). ', ...
            'To make changes to this Scenario first create a deep copy using ', ...
            'scenario.Clone(), then make alterations to the copy.'];
        err_badFCT = ['Cannot apply full-correlator tensor before ',...
                      'MatrixSystem has been generated.'];
    end
    
    %% Construction and initialization
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
            
            obj.matrix_system = MatrixSystem.empty;
        end
        
        function AddParty(obj, name)
            arguments
                obj (1,1) Scenario
                name (1,1) string = ''
            end
            import Scenario.Party
            
            % Check not locked.
            obj.errorIfLocked();
            
            % Add a parrty
            next_id = length(obj.Parties)+1;
            if nargin >=2
                obj.Parties(end+1) = Party(obj, next_id, name);
            else
                obj.Parties(end+1) = Party(obj, next_id);
            end
            obj.invalidateMomentMatrix();
        end
        
        function val = Clone(obj)
            arguments
                obj (1,1) Scenario
            end
            % Construct new scenario
            val = Scenario(0);
            
            % Clone all parties
            for party_id = 1:length(obj.Parties)
                val.Parties(end+1) = obj.Parties(party_id).Clone();
            end
            
        end
        
        function val = get.HasMatrixSystem(obj)
            val = ~isempty(obj.matrix_system);
        end
    end
    
    %% MatrixSystem, MomentMatrices and LocalizingMatrices
    methods
        function val = GetMatrixSystem(obj)
            arguments
                obj (1,1) Scenario
            end
            
            % Make matrix system, if not already generated
            if isempty(obj.matrix_system)
                obj.matrix_system = MatrixSystem(obj);
            end
            
            val = obj.matrix_system;
        end
        
        
        function mm_out = MakeMomentMatrix(obj, depth, skip_bind)
            arguments
                obj (1,1) Scenario
                depth (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                skip_bind (1,1) logical = false
            end
            
            mm_out = MomentMatrix(obj.GetMatrixSystem(), depth);
            if ~skip_bind
                obj.do_bind(mm_out)
            end
        end
    end
    
    %% Accessors and information
    methods
        function val = get.MeasurementsPerParty(obj)
            val = zeros(1, length(obj.Parties));
            for party_id = 1:length(obj.Parties)
                val(party_id) = length(obj.Parties(party_id).Measurements);
            end
        end
        
        function val = get.OperatorsPerParty(obj)
            val = zeros(1, length(obj.Parties));
            for party_id = 1:length(obj.Parties)
                val(party_id) = obj.Parties(party_id).TotalOperators;
            end
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
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);                
                %TODO: Check sufficient depth of MM generated.
            end
            fc = Scenario.FullCorrelator(obj);
            val = fc.linfunc(tensor);
        end
        
        function val = FCIndex(obj, index)
            arguments
                obj (1,1) Scenario
                index (1,:) uint64
            end
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
            end
            fc = Scenario.FullCorrelator(obj);
            val = fc.at(index);
        end
        
        function val = CGTensor(obj, tensor)
            arguments
                obj (1,1) Scenario
                tensor double
            end
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);                
                %TODO: Check sufficient depth of MM generated.
            end
            fc = Scenario.CollinsGisin(obj);
            val = fc.linfunc(tensor);
        end
    end
    
    %% Internal methods
    methods(Access={?Scenario,?Scenario.Party})
        function errorIfLocked(obj)
            if ~isempty(obj.matrix_system)
                error(obj.err_locked);
            end
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

