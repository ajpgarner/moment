classdef RuleBook < handle
    %RULEBOOK Collection of algebraic rules
   
    properties(GetAccess = public, SetAccess = protected)
        Rules = Algebraic.Rule.empty(1,0)
        Hermitian = true;
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
        function obj = RuleBook(initialRules, is_hermitian)
            
            if isa(initialRules, 'Algebraic.Rule')
                obj.Rules = reshape(initialRules, 1, []);
            elseif iscell(initialRules)
                obj.ImportCellArray(initialRules)
            elseif isa(initialRules, 'Algebraic.RuleBook')
                obj.Rules = initialRules.Rules;
                obj.Hermitian = initialRules.Hermitian;
                if nargin >= 2
                    error(['Copy constructor of RuleBook does not ',...
                           'take more than one argument.']);
                end
                return;
            else
                error(['Rules should be provided either as an array of',...
                    ' Algebraic.Rule, or as a cell array.']);
            end
            
            % Hermicity
            if nargin < 2
                is_hermitian = true;
            end
            obj.Hermitian = logical(is_hermitian);
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
            
            % Parameters to pass to npatk
            extra_params = {};
            if verbose
                extra_params{end+1} = 'verbose';
            end
            if obj.Hermitian
                extra_params{end+1} = 'hermitian';
            else
                extra_params{end+1} = 'nonhermitian';
            end
            
            [output, success] = npatk('complete', extra_params{:}, ...
                'limit', max_iterations, ...
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
                end
                obj.is_complete = npatk('complete', 'test', 'quiet', ...
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
                if ~iscell(input) || ~isequal(size(rule), [1, 2])
                    error(obj.err_cellpair);
                end
                obj.Rules(end+1) = Algebraic.Rule(rule{1}, rule{2});
            end
            obj.tested_complete = false;
        end
        
        function val = ExportCellArray(obj)
            val = cell(1, length(obj.Rules));
            for index = 1:length(obj.Rules)
                val{index} = cell(1, 2);
                val{index}{1} = obj.Rules(index).LHS;
                val{index}{2} = obj.Rules(index).RHS;
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
end

