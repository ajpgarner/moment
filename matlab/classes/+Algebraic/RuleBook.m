classdef RuleBook < handle
    %RULEBOOK Collection of algebraic rules.
    %
    % See also: AlgebraicScenario, Algebraic.Rule
   
    properties(GetAccess = public, SetAccess = protected)
        MaxOperators = uint64(0); % The number of operators in the system.
        Rules = Algebraic.Rule.empty(1,0) % The rewrite rules.
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
        function obj = RuleBook(initialRules, max_ops, ....
                                is_hermitian, is_normal)
        % RULEBOOK Constructs a list of rewrite rules.
        %
        % PARAMS:
        %  initial_rules - Algebraic rewrite rules. Can be given as either 
        %                  a cell array, an array of Algebraic.Rule or 
        %                  another Algebraic.RuleBook object.
        %  max_ops - The highest number of operators in the scenario. Set
        %            to 0 to use the highest number found in initial_rules.
        %  is_hermitian - True if fundamental operators are Hermitian.
        %  is_normal - True if fundamental operators are normal.
        %
        % See also: ALGEBRAIC.RULE
            arguments
                initialRules (1,:)
                max_ops (1,1) uint64 = 0
                is_hermitian (1,1) logical = true
                is_normal (1,1) logical = is_hermitian
            end 
            
            if isa(initialRules, 'Algebraic.Rule')
                obj.Rules = reshape(initialRules, 1, []);
            elseif iscell(initialRules)
                obj.ImportCellArray(initialRules)
            elseif isa(initialRules, 'Algebraic.RuleBook')
                obj.MaxOperators = initialRules.MaxOperators;
                obj.Rules = initialRules.Rules;
                obj.Hermitian = initialRules.Hermitian;
                obj.Normal = initialRules.Normal;
                
                if nargin >= 2
                    error(['Copy constructor of RuleBook does not ',...
                           'take more than one input.']);
                end
                return;
            else
                error(['Rules should be provided either as an array of',...
                    ' Algebraic.Rule, or as a cell array.']);
            end
            
            % Get number of operators
            if max_ops <= 0
                 max_ops = obj.highestMentionedOp();
            end
            obj.MaxOperators = uint64(max_ops);
            
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
        function AddRule(obj, new_rule, new_rule_rhs)
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
        %
        % See also: AlgebraicScenario, Algebraic.Rule
        %
            
            % Complain if locked
            obj.errorIfLocked();
            
            % Construct rule from inputs and append to list
            if nargin >= 3
                obj.Rules(end+1) = Algebraic.Rule(new_rule, new_rule_rhs);
            elseif nargin >= 2
                if isa(new_rule, 'Algebraic.Rule')
                    obj.Rules(end+1) = new_rule;
                elseif iscell(new_rule)
                    if ~isequal(size(new_rule), [1, 2])
                        error(obj.err_cellpair);
                    end
                    obj.Rules(end+1) = Algebraic.Rule(new_rule{1}, ...
                                                      new_rule{2});
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
            
            [output, success] = mtk('complete', extra_params{:}, ...
                'limit', max_iterations, ...
                'operators', obj.MaxOperators, ...
                obj.ExportCellArray());
            
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
                obj.is_complete = mtk('complete', 'test', 'quiet', ...
                    'operators', obj.MaxOperators, ...
                    params{:}, obj.ExportCellArray());
                obj.tested_complete = true;
            end
            val = obj.is_complete;
        end
    end

    %% Import/Export cell array
    methods
        function ImportCellArray(obj, input)
        % IMPORTCELLARRAY Converts cell array into array of Algebraic.Rule objects.
        %
        %
            arguments
                obj (1,1) Algebraic.RuleBook
                input cell
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

