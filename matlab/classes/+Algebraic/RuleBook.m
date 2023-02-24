classdef RuleBook < handle
    %RULEBOOK Collection of algebraic rules.
    %
    % See also: AlgebraicScenario, Algebraic.Rule
   
    properties(GetAccess = public, SetAccess = protected)        
        MaxOperators = uint64(0); % The number of operators in the system.
        Rules = Algebraic.Rule.empty(1,0) % The rewrite rules.
        OperatorNames;      % String names of operators, if known.
        Hermitian = true;   % True if fundamental operators are Hermitian.
        Normal = true;      % True if fundamental operators are Normal.
        IsComplete;         % True if the ruleset is confluent.
    end
    
    properties(Access = private)
        tested_complete = false;
        is_complete = false;
        locked = false;
    end
    
    properties(Constant, Access = private)
        err_locked = ['No more changes to this ruleset are possible, ',...
                      'because it has already been associated with a ',...
                      'matrix system.'];
                  
        err_cellpair = ['Each rule in the cell array should be ',...
                        'specified as a pair.'];
                    
        err_badrule = ['Rule should be provided either as an ',...
                       'Algebraic.Rule, a two element cell array, ',...
                       'or two numerical arrays.'];
    end
    
    
    %% Constructor
    methods
        function obj = RuleBook(operators, initialRules, ...
                                is_hermitian, is_normal)
        % RULEBOOK Constructs a list of rewrite rules.
        %
        % SYNTAX
        %   1. book = RuleBook(otherRuleBook)
        %   2. book = RuleBook(ops, rules, is_herm, is_normal)
        %
        % PARAMS (Syntax 2):
        %  operators - Either the number of the highest number operators in 
        %              the scenario, or a list of names.
        %  initial_rules - Algebraic rewrite rules. Can be given as either 
        %                  a cell array, an array of Algebraic.Rule or 
        %                  another Algebraic.RuleBook object.       
        %  is_hermitian - True if fundamental operators are Hermitian.
        %  is_normal - True if fundamental operators are normal.
        %  names_hint - List of operator names, if they exist.
        %
        % See also: ALGEBRAIC.RULE
            arguments
                operators
                initialRules (1,:)
                is_hermitian (1,1) logical = true
                is_normal (1,1) logical = is_hermitian
            end 
            
            % Copy c'tor
            if isa(operators, 'Algebraic.RuleBook')
                obj.MaxOperators = operators.MaxOperators;
                obj.Rules = operators.Rules;
                obj.OperatorNames = operators.OperatorNames;
                obj.Hermitian = operators.Hermitian;
                obj.Normal = operators.Normal;
                
                if nargin >= 2
                    error(['Copy constructor of RuleBook does not ',...
                           'take more than one input.']);
                end
                return;
            end
            
            % Parse operators
            if isnumeric(operators) && isscalar(operators)
                obj.OperatorNames = string.empty(1,0);
                obj.MaxOperators = uint64(operators);                
            elseif isstring(operators)
                obj.OperatorNames = reshape(operators, 1, []);
                obj.MaxOperators = uint64(length(obj.OperatorNames));
            elseif ischar(operators)
                obj.OperatorNames = string(operators(:))';
                obj.MaxOperators = uint64(length(obj.OperatorNames));
            else
                error("Argument 'operators' should either be the maximum"...
                      +" operator number, or a list of operator names.");
            end
            
            % Parse rules
            if isa(initialRules, 'Algebraic.Rule')
                obj.Rules = reshape(initialRules, 1, []);
            elseif iscell(initialRules)
                obj.ImportCellArray(initialRules);
            else
                error(['Rules should be provided either as an array of',...
                       ' Algebraic.Rule, or as a cell array.']);    
            end
            
            % Hermicity and normality
            obj.Hermitian = logical(is_hermitian);
            if is_hermitian && ~is_normal
                error("Hermitian operators must be normal.");
            end
            obj.Normal = logical(is_normal);
        end
    end
    
    %% Add rules
    methods
        function AddRule(obj, new_rule, new_rule_rhs, negate)
        % ADDRULE Add a rule to the rule book.
        % 
        % SYNTAX
        %   1. rb.AddRule(rule)
        %   2. rb.AddRule(lhs, rhs)
        %
        % PARAMS (Syntax 1)
        %   new_rule - An object of type Algebraic.Rule, defining the rule
        %              or a cell array with two elements, the first
        %              defining the left-hand-side of the rule (pattern),
        %              the second the right-hand-side (replacement).
        %
        % PARAMS (Syntax 2)
        %   new_rule     - An array defining the operator string to match. 
        %   new_rule_rhs - An array defining the operator sequence to 
        %                  replace new_rule with.
        %   negate       - Set to true for LHS => -RHS.
        %
        % See also: AlgebraicScenario, Algebraic.Rule
        %
            
            % Complain if locked
            obj.errorIfLocked();
            
            if nargin < 4
                negate = false;
            else
                negate = logical(negate);
            end
            
            % Construct rule from inputs and append to list
            if nargin >= 3
                obj.Rules(end+1) = Algebraic.Rule(new_rule, ...
                                                  new_rule_rhs, negate);
            elseif nargin >= 2
                if isa(new_rule, 'Algebraic.Rule')
                    obj.Rules(end+1) = new_rule;
                elseif iscell(new_rule)
                    if isequal(size(new_rule), [1, 2])
                        obj.Rules(end+1) = Algebraic.Rule(new_rule{1}, ...
                                                          new_rule{2});
                    elseif isequal(size(new_rule), [1, 3]) ...
                            && isequal(new_rule{2}, '-')
                        obj.Rules(end+1) = Algebraic.Rule(new_rule{1}, ...
                                                          new_rule{2}, ...
                                                          true);
                    else
                        error(obj.err_cellpair);
                    end
                else
                    error(obj.err_badrule);
                end
            else
                error(obj.err_badrule);
            end
            
            % Invalidate completeness test
            obj.tested_complete = false;
        end
    end
    
    %% Completion
    methods
        function success = Complete(obj, max_iterations, verbose)
        % COMPLETE Attempt to complete the set of rules.
        %
        % See description in AlgebraicScenario.Complete
        %
        % PARAMS:
        %   max_iterators - The maximum number of new rules to introduce 
        %                   before giving up
        %   verbose - Set to true to output a log of rules introduced and
        %             reduced.
        %
        % See also: ALGEBRAICSCENARIO.COMPLETE
            arguments
                obj (1,1) Algebraic.RuleBook
                max_iterations (1,1) uint64
                verbose (1,1) logical = false
            end
            
            % Complain if locked...
            obj.errorIfLocked();
            
            % Parameters to pass to mtk
            extra_params = {};
            if verbose
                extra_params{end+1} = 'verbose';
            end
            if obj.Hermitian
                extra_params{end+1} = 'hermitian';
            else
                extra_params{end+1} = 'nonhermitian';
                if obj.Normal
                        extra_params{end+1} = 'normal';
                end
            end
            
            if ~isempty(obj.OperatorNames)
                op_arg = obj.OperatorNames;
            else
                op_arg = obj.MaxOperators;
            end
            
            [output, success] = mtk('complete', extra_params{:}, ...
                'limit', max_iterations, ...
                op_arg, obj.ExportCellArray());
            
            obj.ImportCellArray(output);
            obj.tested_complete = true;
            obj.is_complete = success;
        end
        
        function val = get.IsComplete(obj)
            if ~obj.tested_complete
                params = {};
                if obj.Hermitian
                    params{end+1} = 'hermitian';
                else
                    params{end+1} = 'nonhermitian';
                    if obj.Normal
                        params{end+1} = 'normal';
                    end
                end
                
                if ~isempty(obj.OperatorNames)
                    op_arg = obj.OperatorNames;
                else
                    op_arg = obj.MaxOperators;
                end
                
                obj.is_complete = mtk('complete', 'test', 'quiet', ...
                    params{:}, op_arg, obj.ExportCellArray());
                obj.tested_complete = true;
            end
            val = obj.is_complete;
        end
    end

    %% Import/Export cell array
    methods
        function ImportCellArray(obj, input, names_hint)
        % IMPORTCELLARRAY Converts cell array into array of Algebraic.Rule objects.
        %
        % PARAMS
        %   input - The cell array to parse
        %   names_hint - Operator names.
        %
            arguments
                obj (1,1) Algebraic.RuleBook
                input cell
                names_hint (1,:) string = string.empty(1,0)
            end
            obj.errorIfLocked();
            
            input = reshape(input, 1, []);
            obj.Rules = Algebraic.Rule.empty(1,0);
            for index = 1:length(input)
                rule = input{index};
                if ~iscell(input)
                    error(obj.err_cellpair);
                end
                rhs_index = 2;
                if isequal(size(rule), [1, 3])
                    if rule{2} == '-'
                        rhs_index = 3;                        
                    else
                        error(obj.err_cellpair);
                    end
                elseif ~isequal(size(rule), [1, 2])
                    error(obj.err_cellpair);
                end
                lhs = rule{1};
                rhs = rule{rhs_index};
                
                obj.Rules(end+1) = Algebraic.Rule(lhs, rhs);
                
            end
            obj.tested_complete = false;
        end
        
        function val = ExportCellArray(obj)
        % EXPORTCELLARRAY Produces cell array describing rules of rulebook.
        %
        %
            val = cell(1, length(obj.Rules));
            for index = 1:length(obj.Rules)
                if obj.Rules(index).Negated
                    val{index} = cell(1, 3);
                    val{index}{1} = obj.Rules(index).LHS;
                    val{index}{2} = '-';
                    val{index}{3} = obj.Rules(index).RHS;
                else
                    val{index} = cell(1, 2);
                    val{index}{1} = obj.Rules(index).LHS;
                    val{index}{2} = obj.Rules(index).RHS;
                end
            end
        end
    end
    
    %% Locking methods
    methods(Access={?RuleBook, ?AlgebraicScenario})
        function lock(obj)
            obj.locked = true;
        end
        
        function errorIfLocked(obj)
            if obj.locked
                error(obj.err_locked)
            end
        end
    end
    
    %% Private
    methods(Access=private)
        function val = highestMentionedOp(obj)
            error("Not implemented");
        end
    end
end

