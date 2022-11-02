classdef AlgebraicScenario < Scenario
    %ALGEBRAICSCENARIO
    
    properties(GetAccess = public, SetAccess = protected)
        OperatorCount
        IsHermitian
        RuleBook
    end
    
    %% Constructor
    methods
        function obj = AlgebraicScenario(op_count, rules, is_hermitian)
            % Superclass c'tor
            obj = obj@Scenario();
            
            obj.OperatorCount = uint64(op_count);
            
            % Default to hermitian
            if nargin < 3
                is_hermitian = true;
            end
            obj.IsHermitian = is_hermitian;
            
            % Default to empty ruleset
            if nargin < 2
                rules = cell(1,0);
            end            

            obj.RuleBook = Algebraic.RuleBook(rules, is_hermitian);
        end 
    end
    
    %% Friend/interface methods
    methods(Access={?Scenario,?MatrixSystem})
        % Query for a matrix system
        function ref_id = createNewMatrixSystem(obj)
            arguments
                obj (1,1) AlgebraicScenario
            end
            extra_args = cell(1,0);
            if obj.IsHermitian
                extra_args{end+1} = 'hermitian';
            else
                extra_args{end+1} = 'nonhermitian';
            end

            % Call for matrix system
            ref_id = npatk('new_algebraic_matrix_system', ...
                           extra_args{:}, ...
                           obj.OperatorCount, ...
                           obj.RuleBook.ExportCellArray());
                       
            % No further changes to rules allowed...
            obj.RuleBook.lock();
        end
    end

    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) AlgebraicScenario
                mm (1,1) MomentMatrix
            end
        end
    end
end

