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
        % Number of rows in lattice, or just number of qubits in chain.
        NumberOfRows;
        % Number of columns in lattice, or 1 if a chain.
        NumberOfColumns;      
    end
    
    %% Construction and initialization
    methods
        function obj = PauliScenario(dimensions, varargin)
        % Constructs an inflation causal-compatibility scenario.
        % 
        % PARAMS:
        %   dimensions - The number of qubits in chain, or lattice
        %                dimensions.
        %
        % OPTIONAL [KEY,VALUE] PARAMETERS:
        %  tolerance - The multiplier of eps(1) to treat as zero.
            
            % Number of qubits level (or set default)
            if (nargin < 1) || ~isnumeric(dimensions)
                error("Must specify an integer number of qubits.");
            end
            
            % Parse dimensions
            if numel(dimensions) == 1
                assert(dimensions >= 1, ...
                       "Chain length must be a positive integer");
                qubits = uint64(dimensions);
                lattice_mode = false;
            elseif numel(dimensions) == 2
                assert(dimensions(1) >= 1 && dimensions(2) >= 1, ...
                       "Lattice dimensions must be a positive integer");
                rows = uint64(dimensions(1));
                cols = uint64(dimensions(2));
                qubits = uint64(rows * cols);
                lattice_mode = true;
            else
                error("First parameter must be 1 or 2 dimensional")
            end
            
            % Extract arguments of interest
            [pauli_options, other_options] = ...
                MTKUtil.split_varargin_keys( ...
                    ["wrap", "symmetrized"], ...
                    varargin);
            other_options = MTKUtil.check_varargin_keys(["tolerance"], ...
                                                        other_options);
            other_options = [other_options, {"hermitian"}, true];
            
            % Check specific Pauli scenario arguments
            wrap = false;
            symmetrized = false;
            for idx = 1:2:numel(pauli_options)
                switch pauli_options{idx}
                    case 'wrap'
                       wrap = logical(pauli_options{idx+1});
                    case 'symmetrized'
                       symmetrized = logical(pauli_options{idx+1});
                end
            end
            
            % Call Superclass c'tor
            obj = obj@MTKScenario(other_options{:});
            
            % Save number of qubits
            obj.QubitCount = qubits;
            obj.Wrapped = wrap;
            obj.Symmetrized = symmetrized;
            obj.IsLattice = lattice_mode;
            if obj.IsLattice
                obj.NumberOfRows = rows;
                obj.NumberOfColumns = cols;                
            else
                obj.NumberOfRows = obj.QubitCount;
                obj.NumberOfColumns = 1;                
            end
            
            % If symmetrized, then flag that can alias.
            if obj.Symmetrized
                obj.PermitsSymbolAliases = true;
            end
            
        end
    end
    
    %% Accessors
    methods
        function varargout = getAll(obj)
            % Call parent function not w/ 3 outputs
            if nargout ~= 3
                varargout = getAll@MTKScenario(obj);
                return;
            end
            
            varargout = cell(3, 1);
            varargout{1} = obj.sigmaX();
            varargout{2} = obj.sigmaY();
            varargout{3} = obj.sigmaZ();
        end
        
        function val = sigmaX(obj, site)
        % SIGMAX Get Pauli X operator(s).
        % SYNTAX:
        %  1.  x = sigmaX([number])
        %  2.  all_x = sigmaX()
        % RETURNS
        %  1. Pauli X operator of specified qubit.
        %  2. Vector of Pauli X operators for all qubits.
        %
            if (nargin < 2)
               op_numbers = num2cell((1:obj.QubitCount)*3-2)';
               val = MTKMonomial(obj, op_numbers, 1.0);
               return;
            end

            if ~isnumeric(site) || (numel(site)~=1) ...
                || (site <= 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
           
            val = MTKMonomial(obj, (site*3)-2, 1.0);
        end
        
        function val = sigmaY(obj, site)
        % SIGMAY Get Pauli Y operator(s).
        % SYNTAX:
        %  1.  y = sigmaY([number])
        %  2.  all_y = sigmaY()
        % RETURNS
        %  1. Pauli Y operator of specified qubit.
        %  2. Vector of Pauli Y operators for all qubits.
        %
            if (nargin < 2)
               op_numbers = num2cell((1:obj.QubitCount)*3-1)';
               val = MTKMonomial(obj, op_numbers, 1.0);
               return;
            end

            if ~isnumeric(site) || (numel(site)~=1) ...
                || (site <= 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            
            val = MTKMonomial(obj, (site*3)-1, 1.0);
        end
        
        function val = sigmaZ(obj, site)
        % SIGMAZ Get Pauli Z operator(s).
        % SYNTAX:
        %  1.  z = sigmaZ([number])
        %  2.  all_z = sigmaZ()
        % RETURNS
        %  1. Pauli Z operator of specified qubit.
        %  2. Vector of Pauli Z operators for all qubits.
        %
            if (nargin < 2)
               op_numbers = num2cell((1:obj.QubitCount)*3)';
               val = MTKMonomial(obj, op_numbers, 1.0);
               return;
            end

            if ~isnumeric(site) || (numel(site)~=1) ...
                || (site <= 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            val = MTKMonomial(obj, site*3, 1.0);
        end
        
        function val = symmetrize(obj, thing)
        % SYMMETRIZE Apply lattice/chain symmetries to polynomial
        %            
            assert((nargin >= 2) && (isa(thing, 'MTKMonomial') ...
                                  || isa(thing, 'MTKPolynomial')), ...
                "Symmetrization only works on Monomials and Polynomials");
            assert(thing.Scenario == obj, "Mismatched scenarios!");
            assert(thing.IsScalar, "Can only symmetrize scalar objects.");
            
            % Effectively promote monomial to polynomial:
            if isa(thing, 'MTKMonomial')
                cell_input = {thing.OperatorCell};
            else
                cell_input = thing.OperatorCell;
            end
 
            % Call for symmetrization on operator cell
            poly_spec = mtk('lattice_symmetrize', obj.System.RefId, ...
                            cell_input);

            % Construct Polynomial from result
            val = MTKPolynomial.InitFromOperatorPolySpec(obj, {poly_spec});
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
                % Call for lattice matrix system
                ref_id = mtk('pauli_matrix_system', ...
                    [obj.NumberOfRows, obj.NumberOfColumns], named_args{:});
            else
                % Call for chainmatrix system
                ref_id = mtk('pauli_matrix_system', ...
                             obj.QubitCount, named_args{:});                
            end            
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