classdef LocalityScenario < MTKScenario
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
        MeasurementsPerParty % Number of measurements each agent can make.
        TotalMeasurements % Total number of measurements across all parties.
        OperatorsPerParty % Number of operators associated with each agent.
        OutcomesPerMeasurement % Number of outcomes associated with each measurement.
    end
    
    properties(Dependent, GetAccess=public, SetAccess = private)
        CollinsGisin
        FullCorrelator
    end
    
    properties(Access=private)
        cached_CG = Locality.CollinsGisin.empty(0,0);
        cached_FC = Locality.FullCorrelator.empty(0,0);
    end
    
    
    properties(Constant, Access = private)
        err_badFCT_NoMS = [
            'Cannot generate full correlator matrix before ',...
            'MatrixSystem is generated.'];
        
        err_badCGT_NoMS = [
            'Cannot generate Collins-Gisin tensor before ',...
            'MatrixSystem is generated.'];
        
        err_badFCT_dims = ...
            'System dimensions do not admit a full-correlator matrix.';
    end
    
    %% Construction and initialization
    methods
        function obj = LocalityScenario(varargin)
        % LOCALITYSCENARIO Construct a locality scenario.
        % 
        % SYNTAX:
        %  1. LocalityScenario()
        %       Creates an empty scenario (no parties).
        %  2. LocalityScenario(number of parties)
        %       Creates a scenario, with supplied number of parties, 
        %       but no measurements/outcomes.
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
        
            % Parse arguments to standard form
            [desc, opt_idx] = LocalityScenario.parseInputArguments(varargin);
                        
            % Process optional parameters, if any            
            zero_tolerance = 100;
            if opt_idx <= nargin
                options = Util.check_varargin_keys(...
                    ["zero_tolerance"], ...
                    varargin(opt_idx:end));
                for idx = 1:2:numel(options)
                    switch options{idx}
                        case 'zero_tolerance'
                            zero_tolerance = double(options{idx+1});
                    end
                end
            end
           
            % Superclass c'tor
            obj = obj@MTKScenario(zero_tolerance, true, true);
                        
            % Create parties, as supplied
            obj.Parties = Locality.Party.empty(1,0);
            initial_parties = numel(desc);
            if (initial_parties >= 1)
                for x = 1:initial_parties
                    obj.Parties(end+1) = Locality.Party(obj, x);
                    for mmtIndex = 1:numel(desc{x})
                        obj.Parties(x).AddMeasurement(desc{x}(mmtIndex));
                    end
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

    
    %% Dependent accessors
    methods
        function val = get.MeasurementsPerParty(obj)
            val = zeros(1, length(obj.Parties));
            for party_id = 1:numel(obj.Parties)
                val(party_id) = numel(obj.Parties(party_id).Measurements);
            end
        end
        
        function val = get.TotalMeasurements(obj)
            val = 0;
            for party_id = 1:numel(obj.Parties)
                val = val + numel(obj.Parties(party_id).Measurements);
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
    end
    
    %% Cached object accessors
    methods
        function val = get.CollinsGisin(obj)
        % COLLINSGISIN Gets the associated Collins-Gisin tensor.
            
            % Prevent implicit early generation of matrix system.
            if ~obj.HasMatrixSystem
                error(obj.err_badCGT_NoMS);
            end
            
            if isempty(obj.cached_CG)
                obj.cached_CG = Locality.CollinsGisin(obj);
            end
            
            val = obj.cached_CG;
        end
                    
        function val = get.FullCorrelator(obj)
        % FULLCORRELATOR Gets the associated full-correlator matrix
        
            % Prevent implicit early generation of matrix system.
            if ~obj.HasMatrixSystem
                error(obj.err_badFCT_NoMS);
            end
            
            if isnumeric(obj.cached_FC)
                error(obj.err_badFCT_dims)
            end
            
            if isempty(obj.cached_FC)
                try
                    obj.cached_FC = Locality.FullCorrelator(obj);
                catch Exception
                    obj.cached_FC = 0;
                    error(obj.err_badFCT_dims)
                end
            end
            
            val = obj.cached_FC;        
        end
    end
    
    %% Special accessors
    methods        
        function varargout = getMeasurements(obj)
        % GETMEASURMENTS Get every measurement from each party, in order.
            expected_out = obj.TotalMeasurements;
            varargout = cell(1, expected_out);
            
            out_idx = 1;
            for party_id = 1:numel(obj.Parties)
                for mmt_id = 1:numel(obj.Parties(party_id).Measurements)
                    varargout{out_idx} = obj.Parties(party_id).Measurements(mmt_id);
                    out_idx = out_idx + 1;
                end
            end
        end
        
        function item = get(obj, index, indexB)
        % GET Retrieve an object in the locality scenario by index.
        %
        % SYNTAX
        %   1. norm = setting.get();
        %      Gets the Collins-Gisin tensor (i.e. everything probabilistic.)
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
        %   6. joint_out = setting.get([party P, mmt X, outcome A; ...
        %                               party Q, mmt Y, outcome B])
        %   7. joint_out = setting.get(free_indices, fixed_indices);
        %      Gets the joint distribution formed from the measurements
        %      listed in free_indices, joint with the outcomes listed in
        %      fixed_indices. All referred to parties must be different.
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
            % No arguments: return C-G
            if nargin == 1
                item = obj.CollinsGisin;
                return
            end
            
            if nargin == 2
                index = uint64(index);
                if isempty(index)
                    error("If indices are supplied, they must not be empty.");
                end
                dim = size(index);
                if numel(dim) ~= 2
                    error("Indices must be 1x1, Nx2 or Nx3 array.");
                end
                if dim(2) == 1
                    if dim(1) ~= 1
                        error("Party index must be 1x1 array.");
                    end
                    % Quick return, party
                    item = obj.Parties(index);
                    return;
                elseif dim(2) == 2
                    m_idx = uint64(index);
                    o_idx = uint64.empty(0, 3);
                elseif dim(2) == 3
                    m_idx = uint64.empty(0, 2);
                    o_idx = uint64(index);
                else
                    error("Indices must be 1x1, Nx2 or Nx3 array.");
                end                
            elseif nargin == 3
                if ~isempty(index)
                    dimA = size(index);
                    if numel(dimA) ~= 2 || dimA(2) ~= 2
                        error("First set of indices must be Nx2 array.");
                    end
                    m_idx = uint64(index);
                else
                    m_idx = uint64.empty(0, 2);
                end
                
                if ~isempty(indexB)
                    dimB = size(indexB);
                    if numel(dimB) ~= 2 || dimB(2) ~= 3
                        error("Second set of indices must be Nx3 array.");
                    end
                    o_idx = uint64(indexB);
                else
                    o_idx = uint64.empty(0, 3);
                end                
            end
            
            % Nothing specified, raise error
            if isempty(o_idx) && isempty(m_idx)
                error("If indices are supplied, they must not all be empty.");
            end
                     
            % Quick return of single measurement
            if isempty(o_idx) && (size(m_idx, 1) == 1)
                item = obj.Parties(m_idx(1)).Measurements(m_idx(2));
                return;
            end
            
            % Quick return of single outcome
            if isempty(m_idx) && (size(o_idx, 1) == 1)
               item = obj.Parties(o_idx(1)).Measurements(o_idx(2)).Outcomes(o_idx(3));
               return;
            end
            
            % Get measurements
            free = Locality.Measurement.empty(1,0);
            for f = 1:size(m_idx, 1)
                free(end+1) = ...
                    obj.Parties(m_idx(f, 1)).Measurements(m_idx(f, 2));
            end
            
            % Get outcomes
            fixed = Locality.Outcome.empty(1,0);
            for f = 1:size(o_idx, 1)
                fixed(end+1) = ...
                    obj.Parties(o_idx(f, 1)).Measurements(o_idx(f, 2)).Outcomes(o_idx(f, 3));
            end
            
            % Try to make joint object
            item = Locality.JointProbability(obj, free, fixed);
            
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
            
            fc = obj.FullCorrelator;
            val = fc.linfunc(tensor);
        end
        
        function val = FCIndex(obj, indexA, indexB)
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
                indexA (1,1) uint64
                indexB (1,1) uint64
            end
            
            fc = obj.FullCorrelator;
            val = fc(indexA, indexB);           
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
                error(obj.err_badFCT_NoMS);
            end
            fc = Locality.CollinsGisin(obj);
            val = fc.linfunc(tensor);
        end
    end
    
    
    %% Friend/interface methods
    methods(Access={?MTKScenario,?MTKMatrixSystem})
        % Query for a matrix system
        function ref_id = createNewMatrixSystem(obj)
            
            opt_args = {'tolerance', obj.ZeroTolerance};
            
            ref_id = mtk('locality_matrix_system', ...
                length(obj.Parties), ...
                obj.MeasurementsPerParty, ...
                obj.OutcomesPerMeasurement, ...
                opt_args{:});
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function val = operatorCount(obj)
            val = sum(obj.OperatorsPerParty);            
        end
       
        function val = onSetHermitian(obj, old_value, new_value)
            if new_value ~= true
                error("Base operators in LocalityScenarios must always be Hermitian.");
            end
            val = true;
        end
     
        function val = createSolvedScenario(obj, a, b)
            val = SolvedScenario.SolvedLocalityScenario(obj, a, b);
        end
        
        function str = makeOperatorNames(obj)
            str = strings(1, obj.OperatorCount);
            
            format = mtk_locality_format();
            switch format
                case "Natural"                    
                    idx = 1;
                    for party_id = 1:numel(obj.Parties)
                        for mmt_id = 1:numel(obj.Parties(party_id).Measurements)
                            for out_id = 1:(numel(obj.Parties(party_id).Measurements(mmt_id).Outcomes)-1)
                                str(idx) = obj.Parties(party_id).Name + ".";
                                str(idx) = str(idx) + obj.Parties(party_id).Measurements(mmt_id).Name;
                                str(idx) = str(idx) + num2str(out_id-1);
                                idx = idx+1;
                            end
                        end
                    end
                case "Traditional"
                    idx = 1;
                    for party_id = 1:numel(obj.Parties)
                        for mmt_id = 1:numel(obj.Parties(party_id).Measurements)
                            for out_id = 1:(numel(obj.Parties(party_id).Measurements(mmt_id).Outcomes)-1)
                                str(idx) = obj.Parties(party_id).Name;
                                str(idx) = str(idx) + num2str(out_id-1) + "|";
                                str(idx) = str(idx) + obj.Parties(party_id).Measurements(mmt_id).Name;                                
                                idx = idx+1;
                            end
                        end
                    end                    
                otherwise
                    str= makeOperatorNames@MTKScenario(obj);
            end            
        end
    end
    
    %% Static private functions
    methods(Static, Access=private)
        function [desc, optional_idx] = parseInputArguments(args)
        % PARSEINPUTARGUMENTS Canonicalize constructor arguments.
        %
        % See also: LOCALITYSCENARIO.LOCALITYSCENARIO
        %
           
            optional_idx = 0;
            if numel(args) == 0 || Util.is_effectively_str(args{1})
                % RESULT: Syntax 1
                desc = cell(0, 1);
                optional_idx = 1;
            else                
                if 1 == numel(args{1}) && isnumeric(args{1})
                    initial_parties = (args{1});
                    if numel(args) == 1 || Util.is_effectively_str(args{2})
                        % RESULT: Syntax 2
                        optional_idx = 2;
                        desc = cell(initial_parties, 1);
                    elseif numel(args) >= 3 ...
                        && isnumeric(args{2}) && isnumeric(args{3})
                            % RESULT: Syntax 3
                            desc = cell(initial_parties, 1);
                            for partyIndex = 1:initial_parties
                                desc{partyIndex} = args{3}*ones(args{2},1);
                            end                            
                            optional_idx = 4;
                    else
                        error("Invalid input.");
                    end
                elseif isnumeric(args{1}) 
                    % Result: Syntax 4
                    monolith = args{1};                    
                    assert(mod(numel(monolith), 2)==0, ...
                        "Number of output counts should match number of input counts.");
                    initial_parties = uint64(length(monolith)/2);                                       
                    desc = cell(initial_parties,1);
                    for partyIndex = 1:initial_parties
                        desc{partyIndex} = monolith(partyIndex) ...
                            * ones(monolith(initial_parties+partyIndex),1);
                    end
                    optional_idx = 2;
                elseif isa(args{1}, 'cell')
                    % Result: Syntax 5
                    desc = args{1};                    
                    optional_idx = 2;
                else
                    error("Invalid input format.");
                end
            end
       
            assert(iscell(desc));
        end         
        
    end
    
end

