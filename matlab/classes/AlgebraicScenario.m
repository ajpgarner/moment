classdef AlgebraicScenario < MTKScenario
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
% See also: ALGEBRAIC.OPERATORRULEBOOK, ABSTRACT.SCENARIO
    
%% Properties
    properties(GetAccess = public, SetAccess = private)        
        OperatorRulebook % Manages substitution rules for operator strings.
    end
    
    properties(Access = private)
        operator_count;
        listed_operator_names;
    end
        

%% Constructor
    methods
        function obj = AlgebraicScenario(operators, varargin)
        % Creates an algebraic scenario.
        %
        % SYNTAX:
        %      obj = AlgebraicScenario(number)
        %      obj = AlgebraicScenario(number, rules)
        %      obj = AlgebraicScenario(number, key1, value1, ...)
        %      obj = AlgebraicScenario([name list])
        %      obj = AlgebraicScenario([name list], rules)
        %      obj = AlgebraicScenario([name list], key1, value1, ...)
        %
        % PARAMS:
        %  operators - The number of fundamental operators, or an array
        %              of names of operators to define.
        %  rules - Cell array of rewrite rules, or existing Rulebook
        %
        % OPTIONAL [KEY,VALUE] PARAMETERS:
        %  rules - Cell array of rewrite rules on strings of operators.
        %  hermitian - True if fundamental operators are Hermitian.
        %  interleave - True operators are ordered next to their conjugate.
        %  tolerance - The multiplier of eps(1) to treat as zero.
        %  normal - True if fundamental operators are normal.
        %
            
            % Operators always provided as row vector
            operators = reshape(operators, 1, []);

            % Check and parse optional arguments            
            rules = cell(1,0);
            is_normal = true;
                        
            if numel(varargin) == 1
                rules = varargin{1};
                options = cell.empty(1,0);
            else
                param_names = ["rules", "hermitian", "interleave", ...
                               "normal", "tolerance"];
                options = MTKUtil.check_varargin_keys(param_names, varargin);
                exclude_mask = false(size(options));
                for idx = 1:2:numel(varargin)
                    switch varargin{idx}
                        case 'rules'
                            rules = varargin{idx+1};
                            exclude_mask(idx:(idx+1)) = true;
                        case 'normal'
                            is_normal = logical(varargin{idx+1});
                            exclude_mask(idx:(idx+1)) = true;
                    end
                end
                if any(exclude_mask)
                    options = options(~exclude_mask);
                end
            end

            % Call superclass c'tor
            obj = obj@MTKScenario(options{:});
            
            % Process operator input
            if isnumeric(operators) && isscalar(operators)
                obj.listed_operator_names = string.empty(1,0);
                obj.operator_count = uint64(operators);
            elseif isstring(operators)
                obj.listed_operator_names = reshape(operators, 1, []);
                obj.operator_count = uint64(length(obj.listed_operator_names));                  
            elseif ischar(operators)
                obj.listed_operator_names = reshape(string(operators(:)), 1, []);
                obj.operator_count = uint64(length(obj.listed_operator_names));                
            else
                error("Input 'operators' must be set either to the "...
                      +"number of desired operators, or an array of "...
                      +"operator names.");
            end
            
            % Default to normal, except if non-Hermitian, get interleave.
            if obj.IsHermitian && ~is_normal
                error("Hermitian operators must be normal.");
            end
            interleave = true;
            if ~obj.IsHermitian
                interleave = obj.Interleave;
            end
            
            if isa(rules, 'Algebraic.OperatorRulebook')              
                % Check for consistency
                assert(rules.Hermitian == obj.IsHermitian);
                assert(rules.Interleave == interleave);
                assert(rules.MaxOperators == obj.OperatorCount);
      
                % Copy construct existing rulebook
                obj.OperatorRulebook = Algebraic.OperatorRulebook(rules);
            else            
                % Otherwise construct new rulebook
                obj.OperatorRulebook = ...
                    Algebraic.OperatorRulebook(operators, rules, ...
                                              obj.IsHermitian, ...
                                              interleave, ...
                                              is_normal);
            end
        end
        
   
        function val = Clone(obj)
        % CLONE Construct deep copy of scenario (without associated matrices).
            
            % Named or by number operators
            ops_arg = obj.OperatorCount;
            if (~isempty(obj.OperatorNames))
                ops_arg = obj.OperatorNames;
            end
            
            % Make deep copy:
            val = AlgebraicScenario(ops_arg, ...
                                    'rules', obj.OperatorRulebook, ...
                                    'hermitian', obj.IsHermitian, ...
                                    'interleave', obj.Interleave, ...
                                    'normal', obj.IsNormal, ...
                                    'tolerance', obj.ZeroTolerance);
        end
    end
    
%% Utilities
    methods
        function val = asString(obj, sequence)
        % ASSTRING Format operator sequence as a string object
            if isempty(sequence)
                val = "I";
                return
            end
            str_array = obj.OperatorRulebook.ToStringArray(sequence);
            val = join(str_array, " ");
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
            success = obj.OperatorRulebook.Complete(attempts, verbose);
        end
        
    end
       
    %% Virtual methods    
    methods(Access={?MTKScenario,?MTKMatrixSystem})    
        function ref_id = createNewMatrixSystem(obj)        
        % CREATENEWMATRIXSYSTEM Invoke mtk to create matrix system.
            arguments
                obj (1,1) AlgebraicScenario
            end
            
            nams_args = cell(1,0);
            if isempty(obj.listed_operator_names)
                nams_args{end+1} = obj.operator_count;
            else
                nams_args{end+1} = obj.listed_operator_names;
            end
            
            if obj.IsHermitian
                nams_args{end+1} = 'hermitian';
            else
                if obj.Interleave
                    nams_args{end+1} = 'interleaved';
                else
                    nams_args{end+1} = 'bunched';
                end
                
                if obj.OperatorRulebook.Normal
                    nams_args{end+1} = 'normal';
                end
            end
            
            if obj.ZeroTolerance ~= 1.0
                nams_args{end+1} = 'tolerance';
                nams_args{end+1} = double(obj.ZeroTolerance);
            end
            
            % Call for matrix system
            ref_id = mtk('algebraic_matrix_system', ...
                nams_args{:}, ...
                obj.OperatorRulebook.ExportCellArray());
            
            % No further changes to rules allowed...
            obj.OperatorRulebook.lock();
        end
    end
    
    methods(Access=protected)
        function val = operatorCount(obj)
            val = obj.operator_count;
        end
        
        function val = makeOperatorNames(obj)
            if isempty(obj.listed_operator_names)
                val = makeOperatorNames@MTKScenario(obj);
                return
            end
            
            val = obj.listed_operator_names;
            if ~obj.IsHermitian
                conj_str = obj.listed_operator_names + "*";
                if obj.Interleave            
                    val = [val; conj_str];
                    val = reshape(val, 1, []);
                else
                    val = [val, conj_str];
                end
            end
        end
    end
 
end

