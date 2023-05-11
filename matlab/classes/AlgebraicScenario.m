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
% EXAMPLES:
%       /examples/cvx_polynomial.m
%       /examples/nonhermitian_lm.m
%       /examples/yalmip_polynomial.m
%
% See also: ALGEBRAIC.RULEBOOK, ABSTRACT.SCENARIO
    
%% Properties
    properties(GetAccess = public, SetAccess = protected)
        OperatorCount % Number of fundamental operators in scenario.
        IsHermitian   % True if fundamental operators are Hermitian.
        IsNormal      % True if fundamental operators are Normal.
        Interleave    % True if operators are ordered next to their conjugates.
        RuleBook      % Manages substitution rules for operator strings.
        OperatorNames % Names of the fundamental operators.
    end
    
    properties(Dependent, GetAccess = public)
        % Number of operators, taking into acount conjugates
        EffectiveOperatorCount
    end
    
%% Constructor
    methods
        function obj = AlgebraicScenario(operators, rules, ...
                is_hermitian, interleave, is_normal)
            % Creates an algebraic scenario.
            %
            % PARAMS:
            %  operators - The number of fundamental operators, or an array
            %              of names of operators to define.
            %  rules - Cell array of rewrite rules on strings of operators.
            %  is_hermitian - True if fundamental operators are Hermitian.
            %  is_normal - True if fundamental operators are normal.
            %
            arguments
                operators (1,:)
                rules (1,:) = cell.empty(1,0)
                is_hermitian (1,1) logical = true
                interleave (1,1) logical = false
                is_normal (1,1) logical = is_hermitian
            end

            if nargin <= 3
            	is_normal = is_hermitian;
            end

            % Call superclass c'tor
            obj = obj@Abstract.Scenario();
            
            if isnumeric(operators) && isscalar(operators)
                obj.OperatorNames = string.empty(1,0);
                obj.OperatorCount = uint64(operators);
            elseif isstring(operators)
                obj.OperatorNames = reshape(operators, 1, []);
                obj.OperatorCount = uint64(length(obj.OperatorNames));                  
            elseif ischar(operators)
                obj.OperatorNames = reshape(string(operators(:)), 1, []);
                obj.OperatorCount = uint64(length(obj.OperatorNames));                
            else
                error("Input 'operators' must be set either to the "...
                      +"number of desired operators, or an array of "...
                      +"operator names.");
            end
            
            % Default to normal, except if non-Hermitian
            if is_hermitian && ~is_normal
                error("Hermitian operators must be normal.");
            end

            if is_hermitian && interleave 
                error("Interleave mode only makes sense when operators are non-Hermitian.");
            end
            
            obj.IsHermitian = logical(is_hermitian);
            obj.Interleave = logical(interleave);
            obj.IsNormal = logical(is_normal);
            obj.RuleBook = Algebraic.RuleBook(operators, rules, ...
                                              obj.IsHermitian, ...
                                              obj.Interleave, ...
                                              is_normal);
        end
    end
%% Dependent variables
    methods
        function val = get.EffectiveOperatorCount(obj)            
            if obj.IsHermitian
                val = obj.OperatorCount;
            else
                val = 2 * obj.OperatorCount;
            end
        end
    end

%% Set-up / rule manipulation etc.
    methods
        function success = Complete(obj, attempts, verbose)
        % COMPLETE Attempt to complete the set of rules.
        %
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

            if nargin <= 2
            	verbose = false;
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
            
            export_conjugates = false;
            if nargout ~= obj.OperatorCount
                if obj.IsHermitian || nargout ~= 2*obj.OperatorCount
                    error("getAll() expects %d outputs", obj.OperatorCount);
                end
                export_conjugates = true;
            end
            
            varargout = cell(1, nargout);
            if export_conjugates
                for index = 1:(2*obj.OperatorCount)
                    varargout{index} = obj.get(index);
                end
            else
                if obj.Interleave
                    for index = 1:obj.OperatorCount
                        varargout{index} = obj.get((2*index)-1);
                    end
                else
                    for index = 1:obj.OperatorCount
                        varargout{index} = obj.get(index);
                    end
                end
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
            nams_args = cell(1,0);
            if isempty(obj.OperatorNames)
                nams_args{end+1} = obj.OperatorCount;
            else
                nams_args{end+1} = obj.OperatorNames;
            end
            
            if obj.IsHermitian
                nams_args{end+1} = 'hermitian';
            else
                if obj.Interleave
                    nams_args{end+1} = 'interleaved';
                else
                    nams_args{end+1} = 'bunched';
                end
                
                if obj.IsNormal
                    nams_args{end+1} = 'normal';
                end
            end
            
            % Call for matrix system
            ref_id = mtk('new_algebraic_matrix_system', ...
                nams_args{:}, ...
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

