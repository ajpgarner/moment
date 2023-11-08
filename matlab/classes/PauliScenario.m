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
            if obj.QubitCount > 1
                val = reshape(["X"; "Y"; "Z"] + (1:obj.QubitCount), 1, []);
            else
                val = string.empty(1,0);
            end
        end
    end
end