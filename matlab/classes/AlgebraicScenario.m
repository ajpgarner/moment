classdef AlgebraicScenario < Abstract.Scenario
%ALGEBRAICSCENARIO Scenario for operators with monomial substitution rules.
%
% This general-purpose scenario defines a system of (non-commuting) 
% operators. These may be subject to monomial substitution rules, that 
% replace one (sub)string of operators with another. Only substitutions
% that reductions with respect to the 'shortlex' ordering of strings are 
% allowed. For example, for operators a and b, 'aab -> ab', and 'ab -> aa' 
% are reductions, 'a -> b' and 'b -> aa' are not.
%
% 
% See also: ALGEBRAIC.RULEBOOK, ABSTRACT.SCENARIO
    
    properties(GetAccess = public, SetAccess = protected)
        OperatorCount % Number of fundamental operators in scenario.
        IsHermitian   % True if fundamental operators are Hermitian.
        IsNormal      % True if fundamental operators are Normal.
        RuleBook      % Manages substitution rules for operator strings.
    end
    
    %% Constructor
    methods
        function obj = AlgebraicScenario(op_count, rules, ...
                is_hermitian, is_normal)
            % Creates an algebraic scenario.
            %
            % PARAMS:
            %  op_count - The number of fundamental operators.
            %  rules - Cell array of rewrite rules on strings of operators.
            %  is_hermitian - True if fundamental operators are Hermitian.
            %  is_normal - True if fundamental operators are normal.
            %
            arguments
                op_count (1,1) uint64
                rules (1,:) cell = cell(1,0)
                is_hermitian (1,1) logical = true
                is_normal (1,1) logical = is_hermitian
            end
               
            % Call superclass c'tor
            obj = obj@Abstract.Scenario();
            
            obj.OperatorCount = uint64(op_count);
            
            % Default to normal, except if non-Hermitian
            if is_hermitian && ~is_normal
                error("Hermitian operators must be normal.");
            end
            
            obj.IsHermitian = logical(is_hermitian);
            obj.IsNormal = logical(is_normal);
            obj.RuleBook = Algebraic.RuleBook(rules, obj.OperatorCount, ...
                                              is_hermitian, is_normal);
        end
    end
    
    %% Set-up / rule manipulation etc.
    methods
        function success = Complete(obj, attempts, verbose)
        % COMPLETE Attempt to complete the set of rules.
        % Completion is attempted according to a Knuth-Bendix algorithm,
        % such that new rules are introduced until either the ruleset is
        % confluent (i.e. the order of application of rules will not
        % affect the final result) or the maximum number of attempts is
        % exceeded.
        %
        % It is in general not computable whether a given set of rules can 
        % be completed. 
        %
        % See: https://en.wikipedia.org/wiki/Knuth%E2%80%93Bendix_completion_algorithm
        % 
        % PARAMS:
        %   attempts - The maximum number of new rules to introduce.
        %   verbose - Set to true to output a log of rules and reductions
        %             attempted by the algorithm.
        %
        % RETURNS:
        %   Logical true if the (extended) ruleset is confluent afterwards.
            arguments
                obj (1,1) AlgebraicScenario
                attempts (1,1) uint64
                verbose (1,1) logical = false
            end
            
            obj.errorIfLocked();
            success = obj.RuleBook.Complete(attempts, verbose);
        end
        
    end
    
    %% Bind algebraic expressions to MATLAB objects
    methods
        function item = get(obj, operators)
        % GET Return a monomial object associated with an operator string.
        %
        % PARAMS:
        %     operators - The string of operators.
        %
        % RETURNS:
        %     Object of type ALGEBRAIC.MONOMIAL representing the string.
        %
        % See also: ALGEBRAIC.MONOMIAL
        %
            arguments
                obj (1,1) AlgebraicScenario
                operators (1,:) uint64
            end
            
            item = Algebraic.Monomial(obj, operators, 1.0);
        end
        
        function varargout = getAll(obj)
        % GETALL Creates one monomial object per fundamental operator.
        %
        % USAGE:
        %    [x, y, z] = scenario.getAll();
        %  
        % RETURNS:
        %    Number of monomials equal to OperatorCount, one per operator.
        %
        % See also: ALGEBRAIC.MONOMIAL
        %
            arguments
                obj (1,1) AlgebraicScenario
            end
            if obj.OperatorCount == 0
                error("No operators to get.");
            end
            if nargout ~= obj.OperatorCount
                error("getAll() expects %d outputs", obj.OperatorCount);
            end
            
            varargout = cell(1, nargout);
            for index = 1:obj.OperatorCount
                varargout{index} = obj.get(index);
            end
        end
        
        function val = id(obj)
        % ID Creates a monomial object for the identity element.
        %
        % RETURNS:
        %   A monomial element representing identity.
        %
        % See also: ALGEBRAIC.MONOMIAL
            val = obj.get([]);
        end
    end
    
    %% Friend/interface methods
    methods(Access={?Abstract.Scenario,?MatrixSystem})
        
        function ref_id = createNewMatrixSystem(obj)        
        % CREATENEWMATRIXSYSTEM Invoke mtk to create matrix system.
            arguments
                obj (1,1) AlgebraicScenario
            end
            extra_args = cell(1,0);
            if obj.IsHermitian
                extra_args{end+1} = 'hermitian';
            else
                extra_args{end+1} = 'nonhermitian';
                if obj.IsNormal
                    extra_args{end+1} = 'normal';
                end
            end
            
            % Call for matrix system
            ref_id = mtk('new_algebraic_matrix_system', ...
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
        % ONNEWMOMENTMATRIX Called after a moment matrix has been created.
        %
        % PARAMS:
        %       mm - The newly created moment matrix.
        %
            arguments
                obj (1,1) AlgebraicScenario
                mm (1,1) OpMatrix.MomentMatrix
            end
        end
    end
end

