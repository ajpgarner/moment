classdef RuleBook < handle
    %RULEBOOK Collection of algebraic rules
   
    properties(GetAccess = public, SetAccess = protected)
        MaxOperators = uint64(0);
        Rules = Algebraic.Rule.empty(1,0)
        Hermitian = true;
        Normal = true;
        IsComplete;
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
                           'take more than one argument.']);
                end
                return;
            else
                error(['Rules should be provided either as an array of',...
                    ' Algebraic.Rule, or as a cell array.']);
            end
            
            % Get number of operators count
            if nargin < 2
                max_ops = uint64(0);
            else
                max_ops = uint64(max_ops);
            end
            if max_ops <= 0
                 max_ops = obj.highestMentionedOp();
            end
            obj.MaxOperators = max_ops;
            
            
            % Hermicity
            if nargin < 3
                is_hermitian = true;
            end
            obj.Hermitian = logical(is_hermitian);
            
            % Normality
            if nargin < 4
                is_normal = is_hermitian;
            end
            obj.Normal = logical(is_normal);
        end
    end
    
    %% Add rules
    methods
        function AddRule(obj, new_rule, new_rule_rhs)            
            % Complain if locked...
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

