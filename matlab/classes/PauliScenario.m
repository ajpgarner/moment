classdef PauliScenario < MTKScenario
%PAULISCENARIO Scenario for operators that are pauli operators
%%
% See also: MTKSCENARIO

    %% Properties
    properties(GetAccess = public, SetAccess = private)
        % The number of qubits
        QubitCount
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
            
            % Other options
            if ~isempty(varargin)
                options = MTKUtil.check_varargin_keys(["tolerance"], varargin);
            else
                options = cell(1,0);
            end
            options = [options, {"hermitian"}, true];

            % Call Superclass c'tor
            obj = obj@MTKScenario(options{:});
            
            % Save number of qubits
            obj.QubitCount = qubits;
            
        end
    end
    
    %% Accessors
    methods
        function val = sigmaX(obj, site)
            if (nargin < 2) || ~isnumeric(site) ...
                    || (numel(site)~=1) ...
                    || (site < 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            val = MTKMonomial(obj, (site-1)*3+1, 1.0);
        end
        
        function val = sigmaY(obj, site)
            if (nargin < 2) || ~isnumeric(site) ...
                    || (numel(site)~=1) ...
                    || (site < 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            val = MTKMonomial(obj, (site-1)*3+2, 1.0);
        end
        
        function val = sigmaZ(obj, site)
            if (nargin < 2) || ~isnumeric(site) ...
                    || (numel(site)~=1) ...
                    || (site < 0) || (site > obj.QubitCount)
                error("Qubit number must be between 1 and %d.", obj.QubitCount);
            end
            val = MTKMonomial(obj, (site-1)*3+3, 1.0);
        end
    end
    
    %% Overloaded operator matrices and dictionaries
    methods
        function val = WordList(obj, length, nn, wrap, register)
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
                wrap = false;
            else
                assert(numel(wrap) == 1 && islogical(wrap), ...
                   "Wrap flag must be logical scalar (true or false).");
            end
            
            if nargin < 5
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
                    'neighbours', nn, 'wrap', wrap);
                
                obj.System.UpdateSymbolTable();
                
                val = MTKMonomial.InitAllInfo(obj, ops, coefs, hashes, ...
                    symbols, conj, real, im);
            else
                [ops, coefs, hashes] = mtk('word_list', 'monomial',...
                    obj.System.RefId, length, ...
                    'neighbours', nn, 'wrap', wrap);
                val = MTKMonomial.InitDirect(obj, ops, coefs, hashes);
            end
        end
            
        function val = MomentMatrix(obj, level, neighbours, wrap)
        % MOMENTMATRIX Construct a moment matrix for the Pauli scenario.
        %   PARAMS:
        %       level - NPA Hierarchy level
        %       neighbours - If positive, restrict moments to this number 
        %                    of nearest neighbours in top row of matrix.
        %       wrap - Set to true to treat qubit N and 1 as neighbouring.       
        
            % Defaults:
            assert(nargin>=2, "Moment matrix level must be specified.");            
            if nargin < 3
                neighbours = 0;
            end
            if nargin < 4
                wrap = false;
            end
            
            val = Pauli.NNMomentMatrix(obj, level, neighbours, wrap);            
        end
        
      function val = LocalizingMatrix(obj, expr, level, neighbours, wrap)
        % MOMENTMATRIX Construct a moment matrix for the Pauli scenario.
        %   PARAMS:
        %       level - NPA Hierarchy level
        %       expr - The localizing expression
        %       neighbours - If positive, restrict moments to this number 
        %                    of nearest neighbours in top row of matrix.
        %       wrap - Set to true to treat qubit N and 1 as neighbouring.       
        
            % Defaults:
            assert(nargin>=2, "Moment matrix level must be specified.");
            assert(nargin>=3, "Localizing expression must be specified.");
            if nargin < 4
                neighbours = 0;
            end
            if nargin < 5
                wrap = false;
            end
            
            val = Pauli.NNLocalizingMatrix(obj, level, expr, ...
                                           neighbours, wrap);
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