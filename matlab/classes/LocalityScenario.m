classdef LocalityScenario < Abstract.Scenario
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
            %  LocalityScenario([out A, out B, ..., mmts A, mmts B, ...])
            %  LocalityScenario({[out A1, out A2, ..],[out B1, ...]})
            
            % Superclass c'tor
            obj = obj@Abstract.Scenario();
            
            % Create normalization object, and empty system ref
            obj.Parties = Locality.Party.empty;
            
            % How many parties?
            if (nargin == 0)
                initial_parties = uint64(0);
                initialize_mmts = false;
            else
                if 1 == numel(argA)
                    initial_parties = uint64(argA);
                    if nargin == 1
                        initialize_mmts = false;
                    elseif nargin == 3
                        initialize_mmts = true;
                    else
                        error("Invalid input.");
                    end
                elseif isa(argA,'double')
                    initial_parties = uint64(length(argA)/2);
                    initialize_mmts = true;
                elseif isa(argA,'cell')
                    initial_parties = length(argA);
                    initialize_mmts = true;
                end
            end
            
            % Create empty parties
            if (initial_parties >= 1 )
                for x = 1:initial_parties
                    obj.Parties(end+1) = Locality.Party(obj, x);
                end
            end
            
            % Create normalization object
            obj.Normalization = Locality.Normalization(obj);
            
            % No more construction, if no initial measurements
            if ~initialize_mmts
                return
            end
            
            if (nargin == 3)  % Do we have a (party, mmt, outcome) specification?
                desc = cell(initial_parties,1);
                for partyIndex = 1:initial_parties
                    desc{partyIndex} = argC*ones(argB,1);
                end
            elseif isa(argA,'double') %Or we have a [outcomes, measurements] specification?
                desc = cell(initial_parties,1);
                for partyIndex = 1:initial_parties
                    desc{partyIndex} = argA(partyIndex)*ones(argA(initial_parties+partyIndex),1);
                end
            elseif isa(argA,'cell') %Or the general case?
                desc = argA;
            end
            
            % Now, create measurements
            for partyIndex = 1:initial_parties
                for mmtIndex = 1:length(desc{partyIndex})
                    obj.Parties(partyIndex).AddMeasurement(desc{partyIndex}(mmtIndex));
                end
            end
        end
        
        function AddParty(obj, name)
            arguments
                obj (1,1) LocalityScenario
                name (1,1) string = string.empty(1,0)
            end
            
            % Check not locked.
            obj.errorIfLocked();
            
            % Add a party
            next_id = length(obj.Parties)+1;
            if nargin >=2
                obj.Parties(end+1) = Locality.Party(obj, next_id, name);
            else
                obj.Parties(end+1) = Locality.Party(obj, next_id);
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
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
            end
            fc = Locality.FullCorrelator(obj);
            val = fc.linfunc(tensor);
        end
        
        function val = FCIndex(obj, index)
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
            end
            fc = Locality.FullCorrelator(obj);
            val = fc.at(index);
        end
        
        function val = CGTensor(obj, tensor)
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
            end
            fc = Locality.CollinsGisin(obj);
            val = fc.linfunc(tensor);
        end
    end
    
    %% Internal methods
    methods(Access={?LocalityScenario,?Locality.Party})
        function make_joint_mmts(obj, party_id, new_mmt)
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
    methods(Access={?Abstract.Scenario,?MatrixSystem})
        % Query for a matrix system
        function ref_id = createNewMatrixSystem(obj)
            ref_id = mtk('new_locality_matrix_system', ...
                length(obj.Parties), ...
                obj.MeasurementsPerParty, ...
                obj.OutcomesPerMeasurement);
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
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
        
        function val = createSolvedScenario(obj, a, b)
            val = SolvedScenario.SolvedLocalityScenario(obj, a, b);
        end
    end
end

