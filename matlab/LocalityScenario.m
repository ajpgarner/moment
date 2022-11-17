classdef LocalityScenario < Scenario
    %LOCALITYSCENARIO Disjoint agents with projective measurements.
    %
    
    properties(GetAccess = public, SetAccess = protected)
        Parties
        Normalization
        MeasurementsPerParty
        OperatorsPerParty
        OutcomesPerMeasurement
    end
    
    properties(Constant, Access = protected)
        err_badFCT = [
            'Cannot apply full-correlator tensor before ',...
            'MatrixSystem has been generated.'];
    end
    
    %% Construction and initialization
    methods
        function obj = LocalityScenario(argA, argB, argC)
            % Constructs a locality scenario.
            % Possible syntaxes:
            %  LocalityScenario()
            %  LocalityScenario(number of parties)
            %  LocalityScenario(number of parties, mmts per party, outcomers per mmt)
            %  LocalityScenario([mmts A, mmts B, ...], [out A, out B, ...])
            %  LocalityScenario([mmts A, mmts B, ...], [out A1, out A2, .. out B1, ...])
            
            % Superclass c'tor
            obj = obj@Scenario();
            
            % Create normalization object, and empty system ref
            obj.Parties = Locality.Party.empty;
            
            % How many parties?
            if (nargin < 1)
                initial_parties = uint64(0);
            else
                if 1 == numel(argA)
                    if (nargin == 1) || (nargin >= 3)
                        initial_parties = uint64(argA);
                    else
                        initial_parties = 1;
                    end
                else
                    if (nargin >=3)
                        error("If three arguments are provided, number "...
                            + "of parties should be a scalar.");
                    end
                    initial_parties = uint64(length(argA));
                end
            end
            
            % Create empty parties
            if (initial_parties >=1 )
                for x = 1:initial_parties
                    obj.Parties(end+1) = Locality.Party(obj, x);
                end
            end
            
            % Create normalization object
            obj.Normalization = Locality.Normalization(obj);
            
            % Are we also setting up measurements?
            initialize_mmts = (nargin >= 2);
            
            % No more construction, if no initial measurements
            if ~initialize_mmts
                return
            end
            
            % Do we have a (party, mmt, outcome) specification?
            pmo_specified = (nargin >= 3);
            if pmo_specified
                mmt_input = uint64(argB);
                out_input = uint64(argC);
            else
                mmt_input = uint64(argA);
                out_input = uint64(argB);
            end
            
            % Get measurements per party...
            if numel(mmt_input) > 1
                if pmo_specified && (numel(mmt_input) ~= initial_parties)
                    error("Number of measurements should either be a "...
                        + "scalar, or an array with as many elements "...
                        + "as parties.");
                end
                mmts_per_party = mmt_input;
            else
                mmts_per_party = uint64(ones(1, initial_parties)) ...
                    * mmt_input;
            end
            total_mmts = sum(mmts_per_party);
            
            
            % Get outcomes per measurement
            if numel(out_input) == 1
                outputs_per_mmt = uint64(ones(1, total_mmts)) * out_input;
            else
                if numel(out_input) == initial_parties
                    outputs_per_mmt = uint64.empty(1,0);
                    for i = 1:initial_parties
                        outputs_per_mmt = horzcat(outputs_per_mmt, ...
                            uint64(ones(1, mmts_per_party(i))) ...
                            * out_input(i));
                    end
                elseif numel(out_input) == total_mmts
                    outputs_per_mmt = out_input;
                else
                    error("Number of outputs should be given either as "...
                        + "a scalar, an array with as many elements as "...
                        + "parties, or an array with as many elements "...
                        + "as total number of measurements.");
                end
            end
            
            % Now, create measurements
            outIndex = 1;
            for partyIndex = 1:initial_parties
                for mmtIndex = 1:mmts_per_party(partyIndex)
                    obj.Parties(partyIndex).AddMeasurement(...
                        outputs_per_mmt(outIndex));
                    outIndex = outIndex + 1;
                end
            end
        end
        
        function AddParty(obj, name)
            arguments
                obj (1,1) LocalityScenario
                name (1,1) string = ''
            end
            import Locality.Party
            
            % Check not locked.
            obj.errorIfLocked();
            
            % Add a party
            next_id = length(obj.Parties)+1;
            if nargin >=2
                obj.Parties(end+1) = Party(obj, next_id, name);
            else
                obj.Parties(end+1) = Party(obj, next_id);
            end
        end
        
        function val = Clone(obj)
            arguments
                obj (1,1) LocalityScenario
            end
            % Construct new LocalityScenario
            val = LocalityScenario(0);
            
            % Clone all parties
            for party_id = 1:length(obj.Parties)
                val.Parties(end+1) = obj.Parties(party_id).Clone();
            end
            
        end
        
    end
    
    
    %% Overloaded accessor: MatrixSystem
    methods
        function val = System(obj)
            arguments
                obj (1,1) LocalityScenario
            end
            
            % Make matrix system, if not already generated
            if isempty(obj.matrix_system)
                obj.matrix_system = Locality.LocalityMatrixSystem(obj);
            end
            
            val = obj.matrix_system;
        end     
    end
    
    %% Locality accessors and information
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
        
        function val = get.OutcomesPerMeasurement(obj)
            total_m = sum(obj.MeasurementsPerParty);
            val = zeros(1, length(total_m));
            index = 1;
            for party = obj.Parties
                for mmt = party.Measurements
                    val(index) = length(mmt.Outcomes);
                    index = index + 1;
                end
            end
        end
        
        function item = get(obj, index)
            arguments
                obj (1,1) LocalityScenario
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
                obj (1,1) LocalityScenario
                tensor double
            end
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
                %TODO: Check sufficient depth of MM generated.
            end
            fc = Locality.FullCorrelator(obj);
            val = fc.linfunc(tensor);
        end
        
        function val = FCIndex(obj, index)
            arguments
                obj (1,1) LocalityScenario
                index (1,:) uint64
            end
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
            end
            fc = Locality.FullCorrelator(obj);
            val = fc.at(index);
        end
        
        function val = CGTensor(obj, tensor)
            arguments
                obj (1,1) LocalityScenario
                tensor double
            end
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
                %TODO: Check sufficient depth of MM generated.
            end
            fc = Locality.CollinsGisin(obj);
            val = fc.linfunc(tensor);
        end
    end
    
    %% Internal methods
    methods(Access={?LocalityScenario,?Locality.Party})
        function make_joint_mmts(obj, party_id, new_mmt)
            arguments
                obj (1,1) LocalityScenario
                party_id (1,1) uint64
                new_mmt (1,1) Locality.Measurement
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
    
    %% Friend/interface methods
    methods(Access={?Scenario,?MatrixSystem})
        % Query for a matrix system
        function ref_id = createNewMatrixSystem(obj)
            arguments
                obj (1,1) LocalityScenario
            end
            ref_id = npatk('new_locality_matrix_system', ...
                           length(obj.Parties), ...
                           obj.MeasurementsPerParty, ...
                           obj.OutcomesPerMeasurement);
        end
    end
    
    
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) LocalityScenario
                mm (1,1) MomentMatrix
            end
            p_table = mm.MatrixSystem.ProbabilityTable;
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
                    joint_outcome = Locality.JointOutcome(obj, p_row.indices);
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
