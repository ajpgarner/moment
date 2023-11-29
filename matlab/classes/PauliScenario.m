classdef PauliScenario < MTKScenario
%PAULISCENARIO Scenario for operators that are pauli operators
%%
% See also: MTKSCENARIO

    %% Properties
    properties(GetAccess = public, SetAccess = private)
        % The number of qubits
        QubitCount
        % True, if system should wrap / tile
        Wrapped
        % True, if system has translational symmetry
        Symmetrized
        % True, if the system is a lattice
        IsLattice;
        % Number of columns in lattice, or just number of qubits in chain.
        NumberOfColumns;
        % Number of rows in lattice, or just 1 if a chain.
        NumberOfRows;
    end
    
    %% Construction and initialization
    methods
        function obj = PauliScenario(qubits, varargin)
        % Constructs an inflation causal-compatibility scenario.
        % 
        % PARAMS:
        %   qubits - The number of qubits
        %
        % OPTIONAL [KEY,VALUE] PARAMETERS:
        %  tolerance - The multiplier of eps(1) to treat as zero.
            
            % Number of qubits level (or set default)
            if (nargin < 1) || ~isnumeric(qubits) ...
                    || (numel(qubits)~=1) || (qubits < 0)
                error("Must specify a positive scalar integer number of qubits.");
            else
                qubits = uint64(qubits);
            end
            
            % Extract arguments of interest
            [pauli_options, other_options] = ...
                MTKUtil.split_varargin_keys( ...
                    ["wrap", "symmetrized", "lattice", "columns"], ...
                    varargin);
            other_options = MTKUtil.check_varargin_keys(["tolerance"],...
                                                        other_options);
            other_options = [other_options, {"hermitian"}, true];
            
            % Check specific Pauli scenario arguments
            wrap = false;
            symmetrized = false;
            set_lattice = false;
            lattice = false;
            columns = 0;
            for idx = 1:2:numel(pauli_options)
                switch pauli_options{idx}
                    case 'wrap'
                       wrap = logical(pauli_options{idx+1});
                    case 'symmetrized'
                        symmetrized = logical(pauli_options{idx+1});
                    case 'lattice'
                       set_lattice = true;
                       lattice = logical(pauli_options{idx+1});
                    case 'columns'
                       columns = uint64(pauli_options{idx+1});
                end
            end
            
            % Check for inconsistencies with lattice specification
            if lattice
                if columns == 0
                    columns = uint64(sqrt(double(qubits)));
                    assert(columns * columns == qubits, ...
                        "If lattice mode is set and no column number is provided," ...
                       + " number of qubits should be a square number.");
                    
                end
            elseif columns > 1
                if (set_lattice && ~lattice)
                    assert(columns~=0, ...
                        "If lattice is false, no column parameter should be set.");
                end
                lattice = true;
            end            
            assert(~lattice || (mod(qubits, columns) == 0), ...
                "Number of qubits should be divisible by number of columns");

            % Call Superclass c'tor
            obj = obj@MTKScenario(other_options{:});
            
            % Save number of qubits
            obj.QubitCount = qubits;
            obj.Wrapped = wrap;
            obj.Symmetrized = symmetrized;
            obj.IsLattice = lattice;
            if obj.IsLattice
                obj.NumberOfColumns = columns;
                obj.NumberOfRows = uint64(obj.QubitCount / columns);
            else
                obj.NumberOfColumns = obj.QubitCount;
                obj.NumberOfRows = 1;
            end
            
            % If symmetrized, then flag alias warning
            if obj.Symmetrized
                obj.PermitsSymbolAliases = true;
            end
            
        end
    end
    
    %% Accessors
    methods
        function val = sigmaX(obj, site)
            if (nargin < 2) || ~isnumeric(site) ...
                    || (numel(site)~=1) ...
                    || (site <= 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            val = MTKMonomial(obj, (site-1)*3+1, 1.0);
        end
        
        function val = sigmaY(obj, site)
            if (nargin < 2) || ~isnumeric(site) ...
                    || (numel(site)~=1) ...
                    || (site <= 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            val = MTKMonomial(obj, (site-1)*3+2, 1.0);
        end
        
        function val = sigmaZ(obj, site)
            if (nargin < 2) || ~isnumeric(site) ...
                    || (numel(site)~=1) ...
                    || (site <= 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            val = MTKMonomial(obj, (site-1)*3+3, 1.0);
        end
    end
    
    %% Overloaded operator matrices and dictionaries
    methods
        function val = WordList(obj, length, nn, register)
            default_wl = false;
            if nargin < 2 || ~isnumeric(length) || length < 0
                error("Must specify a positive integer length.");
            else
                length = uint64(length);
            end

            % Special 'overload' if nn is true or false, or not specified
            if nargin < 3
                register = false;
                default_wl = true;
            elseif islogical(nn)
                register = nn;
                default_wl = true;
            end
            
            if default_wl
                val = WordList@MTKScenario(obj, length, register);
                return;                
            end
             
            if nargin < 4
                register = false;
            else
                assert(numel(register) == 1 && islogical(register), ...
                    "Register flag must be logical scalar (true or false).");
                register = logical(register);
            end
            
            if register
                [ops, coefs, hashes, symbols, conj, real, im] = ...
                    mtk('word_list', 'register_symbols', 'monomial', ...
                    obj.System.RefId, length, ...
                    'neighbours', nn);
                
                obj.System.UpdateSymbolTable();
                
                val = MTKMonomial.InitAllInfo(obj, ops, coefs, hashes, ...
                    symbols, conj, real, im);
            else
                [ops, coefs, hashes] = mtk('word_list', 'monomial',...
                    obj.System.RefId, length, ...
                    'neighbours', nn);
                val = MTKMonomial.InitDirect(obj, ops, coefs, hashes);
            end
        end
            
        function val = MomentMatrix(obj, level, neighbours)
        % MOMENTMATRIX Construct a moment matrix for the Pauli scenario.
        %   PARAMS:
        %       level - NPA Hierarchy level
        %       neighbours - If positive, restrict moments to this number 
        %                    of nearest neighbours in top row of matrix.     
        
            % Defaults:
            assert(nargin>=2, "Moment matrix level must be specified.");            
            if nargin < 3
                neighbours = 0;
            end
            
            val = Pauli.NNMomentMatrix(obj, level, neighbours);            
        end
        
      function val = LocalizingMatrix(obj, expr, level, neighbours)
       % LOCALIZINGMATRIX Construct a moment matrix for the Pauli scenario.
       %   PARAMS:
       %       level - NPA Hierarchy level
       %       expr - The localizing expression
       %       neighbours - If positive, restrict moments to this number 
       %                    of nearest neighbours in top row of matrix.
        
            % Defaults:            
            assert(nargin>=2, "Localizing expression must be specified.");
            assert(nargin>=3, "Hierarchy level must be specified.");
            if nargin < 4
                neighbours = 0;
            end
            
            val = Pauli.NNLocalizingMatrix(obj, level, expr, neighbours);
      end
      
      function val = CommutatorMatrix(obj, expr, level, neighbours)
      % COMMUTATORMATRIX Calculate the commutator of moment matrix with 
      %                  expression.
      %   PARAMS:
      %       level - NPA Hierarchy level.
      %       expr - The expression to commute with.
      %       neighbours - If positive, restrict moments to this number 
      %                    of nearest neighbours in top row of matrix.
             
          % Defaults:          
          assert(nargin>=2, "Expression to commute with must be specified.");
          assert(nargin>=3, "Hierarchy level must be specified.");
          if nargin < 4
              neighbours = uint64(0);
          end
          val = Pauli.CommutatorMatrix(obj, level, expr, neighbours);
      end
      
      
      function val = AnticommutatorMatrix(obj, expr, level, neighbours)
      % ANTICOMMUTATORMATRIX Calculate the anti-commutator of moment matrix 
      %                      with expression.
      %   PARAMS:
      %       level - NPA Hierarchy level.
      %       expr - The expression to anti-commute with.
      %       neighbours - If positive, restrict moments to this number 
      %                    of nearest neighbours in top row of matrix.
       
          % Defaults:          
          assert(nargin>=2, "Expression to anti-commute with must be specified.");
          assert(nargin>=3, "Hierarchy level must be specified.");
          if nargin < 4
              neighbours = uint64(0);
          end
          val = Pauli.AnticommutatorMatrix(obj, level, expr, neighbours);
      end
    end
           
    %% Virtual methods    
    methods(Access={?MTKScenario,?MTKMatrixSystem})    
        function ref_id = createNewMatrixSystem(obj)        
        % CREATENEWMATRIXSYSTEM Invoke mtk to create matrix system.
            
            named_args = cell(1,0);
            if obj.ZeroTolerance ~= 1.0
                named_args{end+1} = 'tolerance';
                named_args{end+1} = double(obj.ZeroTolerance);
            end
            if obj.Wrapped
                named_args{end+1} = 'wrap';
            end
            if obj.Symmetrized
                named_args{end+1} = 'symmetrized';
            end
            if obj.IsLattice
                named_args{end+1} = 'columns';
                named_args{end+1} = obj.NumberOfColumns;
            end
            % Call for matrix system
            ref_id = mtk('pauli_matrix_system', ...
                obj.QubitCount, named_args{:});            
        end
    end
    
    methods(Access=protected)
        function val = operatorCount(obj)
            val = obj.QubitCount * 3;
        end
        
        function val = makeOperatorNames(obj)
            if obj.QubitCount >= 1
                val = reshape(["X"; "Y"; "Z"] + (1:obj.QubitCount), 1, []);
            else
                val = string.empty(1,0);
            end
        end
    end
end