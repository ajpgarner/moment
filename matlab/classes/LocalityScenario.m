classdef LocalityScenario < Abstract.Scenario
%LOCALITYSCENARIO Scenario for agents with projective measurements.
%
% This scenario is the classic locality setting, of spatially-disjoint
% agents (Alice, Bob, etc.) who choose to make a measurement, and record the 
% output.
%
% All measurements are assumed to be projective and complete, such that
% each measurement with N outcomes defines N-1 fundamental operators. For
% operators Xi, Xj within a measurement, it is taken that XiXj = Xi if i=j 
% and XiXj = 0 otherwise. Operators belonging to different measurements 
% made by the same party are assumed not to commute. Operators from 
% measurements made by different parties commute.
%    
% The moment matrices generated from such a scenario implement the NPA
% hierarchy. [See: https://doi.org/10.1088/1367-2630/10/7/073013].
%
% The scenario can either be specified entirely in the constructor, or
% an empty scenario can be created and parties and measurements be
% added via the AddParty method (and the AddMeasurement method of
% Locality.Party).
%
% EXAMPLES:
%       /examples/chsh.m
%       /examples/cvx_chsh.m
%       /examples/cvx_I3322.m
%       /examples/four_party.m
%       /examples/three_party.m
%       /examples/yalmip_chsh.m
%
% See also: Locality.Party, Locality.Measurement, Locality.Outcome,
%           SolvedScenario.SolvedLocalityScenario
%
    
    properties(GetAccess = public, SetAccess = protected)
        Parties % Spatially disjoint agents (Alice, Bob, etc.).
        Normalization % Normalization element of scenario (i.e. identity matrix).
        MeasurementsPerParty % Number of measurements each agent can make.
        OperatorsPerParty % Number of operators associated with each agent.
        OutcomesPerMeasurement % Number of outcomes associated with each measurement.
    end
    
    properties(Constant, Access = protected)
        err_badFCT = [
            'Cannot apply full-correlator tensor before ',...
            'MatrixSystem has been generated.'];
    end
    
    %% Construction and initialization
    methods
        function obj = LocalityScenario(argA, argB, argC)
            % LOCALITYSCENARIO Construct a locality scenario.
            % 
            % SYNTAX:
            %  1. LocalityScenario()
            %       Creates an empty scenario (no parties).
            %  2. LocalityScenario(number of parties)
            %       Creates a scenario, with a number of parties, but no
            %       measurements/outcomes.
            %  3. LocalityScenario(number of parties, mmts per party, outcomers per mmt)
            %       Creates a scenario, which each party has the same
            %       number of measurements, and each measurement the same
            %       number of outcomes.
            %  4. LocalityScenario([out A, out B, ..., mmts A, mmts B, ...])
            %       Creates a scenario where each party may have different
            %       numbers of measurements/outcomes, but within any
            %       particular party, all measurements have the same number
            %       of outcomes.
            %  5. LocalityScenario({[out A1, out A2, ..], [out B1, ...], ...})
            %       Creates a scenario where each party can have a
            %       different number of measurements, and each measurement
            %       can have a different number of outcomes.
            %
            % See also: AddParty, Party.AddMeasurement
            %
            
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
        % ADDPARTY Add an empty party to the scenario.
        %
        % PARAMS:
        %   name (optional) - The string to identify this party (e.g. "A").
        %                     If blank, parties will be named in
        %                     increasing alphabetical order.
        %
        % RETURNS:
        %   A Locality.Party object, representing the newly added party.
        %
        % See also: Locality.Party
        %
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
            % SYSTEM Gets associated Locality.LocalityMatrixSystem.
            %
            % Will generate the MatrixSystem if it has not yet been
            % created.
            %
            % RETURN:
            %   A Locality.LocalityMatrixSystem.object.
            %
            % See also: Locality.LocalityMatrixSystem.
            %
            
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
        % GET Retrieve an object in the locality scenario by index.
        %
        % SYNTAX
        %   1. norm = setting.get();
        %      Gets the normalization associated with 
        %   2. party = setting.get([party P])
        %      Gets the party at index P.
        %   3. mmt = setting.get([party P, mmt X])
        %      Gets measurement X associated with party A.
        %   4. out = setting.get([party P, mmt X, outcome A])
        %      Gets outcome A associated with measurement X of party A.
        %   5. joint_mmt = setting.get([[party P, mmt X]; [party Q, mmt Y]])
        %      Gets the joint measurement formed from measurement X of
        %      party P together with measurement Y of party Q. P must not
        %      be equal to Q. Multiple parties can be specified, but they
        %      must all be different.
        %   6. joint_out = setting.get([[party P, mmt X, outcome A]; ...
        %                               [party Q, mmt Y, outcome B]])
        %      Gets the joint outcome formed from outcome A of measurement
        %      X of party P with outcome B of measurement Y of party Q.
        %      Multiple parties can be specified, but they must all be
        %      different.
        %
        % PARAMS
        %   index The index/indices of the outcome/measurement (as above).
        %
        % RETURNS
        %   For each syntax 1. Locality.Normalization, 2. Locality.Party, 
        %   3. Locality.Measurement, 4. Locality.Outcome,
        %   5. Locality.JointMeasurement, 6. Locality.JointOutcome.
        %
        % See also: Abstract.RealObject
        %
            arguments
                obj (1,1) LocalityScenario
                index (:,:) uint64 = uint64.empty(1,0);
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
        % FCTENSOR Build an objective function by supplying weights to a full correlator.
        %
        % For example, in a (2,2,2)-CHSH scenario, an object representing 
        % the usual CHSH inequality <A1B1> + <A1B2> + <A2B1> - <A2B2> is 
        % generated by:
        %   CHSH_ineq = scenario.FCTensor([[0 0 0]; [0 1 1]; [0 1 -1]])
        %
        % May throw an error if a moment matrix large enough to contain
        % all correlations has not yet been generated. For bipartite
        % systems, a level 1 matrix is required; for tri- and 4-partite
        % systems, a level 2 matrix is required, etc.
        %
        % RETURNS:
        %   An Abstract.RealObject representing the requested objective
        %   function.
        %
        % See also: Locality.FullCorrelator, Abstract.RealObject
        %
            arguments
                obj (1,1) LocalityScenario
                tensor
            end
            
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT);
            end
            fc = Locality.FullCorrelator(obj);
            val = fc.linfunc(tensor);
        end
        
        function val = FCIndex(obj, index)
        % FCINDEX Retrieve an object corresponding an index in the full-correlator tensor.
        %
        % For a bipartite example, setting the value of index to:
        %    - [1,1] returns a Locality.Normalization
        %    - [x>1, 1] returns the Locality.Measurement for measurement 
        %       X-1 from the first party
        %    - [1, x>1] returns the Locality.Measurement for measurement 
        %       X-1 from the second party
        %    - [x>1,y>1] would return a Locality.JointMeasurement for the
        %    joint measurement of X-1 from the first party and Y-1 from the
        %    second party.
        %
        % For scenarios with more parties, more indices must be set.
        %
        % PARAMS
        %   index - The indices of the element within the full-correlator.
        %
        % RETURNS
        %   As appropriate above, either Locality.Normalization,
        %   Locality.Measurement or Locality.JointMeasurement.
        %
        % See also: Locality.FullCorrelator
        %
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
        % CGTENSOR Build an objective function by supplying weights to a Collins-Gisin tensor.
        %
        % For example, in a (2,2,2)-CHSH scenario, an object representing 
        % the usual CHSH inequality <A1B1> + <A1B2> + <A2B1> - <A2B2> is 
        % generated by:
        %	CHSH_ineq = scenario.CGTensor([[2 -4 0]; [-4 4 4]; [0 4 -4]]))
        %
        % May throw an error if a moment matrix large enough to contain
        % all joint operator strings has not yet been generated. For 
        % bipartite systems, a level 1 matrix is required; for tri- and 
        % 4-partite systems, a level 2 matrix is required, etc.
        %
        % RETURNS:
        %   An Abstract.RealObject representing the requested objective
        %   function.
        %
        % See also: Locality.CollinsGisin, Abstract.RealObject
        %
        arguments
            obj (1,1) LocalityScenario
            tensor double
        end
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
            ref_id = mtk('locality_matrix_system', ...
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

