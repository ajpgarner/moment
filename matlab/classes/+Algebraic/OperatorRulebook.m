classdef OperatorRulebook < handle
    %OPERATORRULEBOOK Collection of algebraic operator rewrite rules.
    %
    % See also: AlgebraicScenario, Algebraic.OperatorRule
   
    properties(GetAccess = public, SetAccess = protected)        
        MaxOperators = uint64(0); % The number of operators in the system.
        Rules = Algebraic.OperatorRule.empty(1,0) % The rewrite rules.
        OperatorNames;      % String names of operators, if known.
        Hermitian = true;   % True if fundamental operators are Hermitian.
        Interleave = false; % True if operators are ordered next to their conjugates.
        Normal = true;      % True if fundamental operators are Normal.
        IsComplete;         % True if the ruleset is confluent.
        extended_names;
    end
    
    properties(Access = private)
        tested_complete = false;
        is_complete = false;
        locked = false;
        
        mono_names = false;
    end
    
    properties(Constant, Access = private)
        err_locked = ['No more changes to this ruleset are possible, ',...
                      'because it has already been associated with a ',...
                      'matrix system.'];
                  
        err_cellpair = ['Each rule in the cell array should be ',...
                        'specified as a pair.'];
                    
        err_badrule = ['Rule should be provided either as an ',...
                       'Algebraic.OperatorRule, a two element cell array, ',...
                       'or two numerical arrays.'];
    end
    
    
    %% Constructor
    methods
        function obj = OperatorRulebook(operators, initialRules, ...
                                        is_hermitian, interleave, is_normal)
        % OPERATORRULEBOOK Constructs a list of rewrite rules.
        %
        % SYNTAX
        %   1. book = OperatorRulebook(otherRulebook)
        %   2. book = OperatorRulebook(ops, rules, is_herm, is_normal)
        %
        % PARAMS (Syntax 2):
        %  operators - Either the number of the highest number operators in 
        %              the scenario, or a list of names.
        %  initial_rules - Algebraic rewrite rules. Can be given as either 
        %                  a cell array, an array of Algebraic.OperatorRule
        %                  or another Algebraic.OperatorRulebook object.    
        %  is_hermitian - True if fundamental operators are Hermitian.
        %  is_normal - True if fundamental operators are normal.
        %  names_hint - List of operator names, if they exist.
        %
        % See also: ALGEBRAIC.OPERATORRULE
        
            assert(nargin>=1)        
            % Copy c'tor
            if isa(operators, 'Algebraic.OperatorRulebook')
                obj.MaxOperators = operators.MaxOperators;
                obj.Rules = operators.Rules;
                obj.OperatorNames = operators.OperatorNames;
                obj.Hermitian = operators.Hermitian;
                obj.Interleave = operators.Interleave;
                obj.Normal = operators.Normal;
                
                if nargin >= 2
                    error(['Copy constructor of OperatorRulebook',...
                           ' does not take more than one input.']);
                end
                return;
            end
            
            if nargin < 3
                is_hermitian = true;
            end
            if nargin < 4 
                interleave = true;
            end
            if nargin < 5
                is_normal = is_hermitian;
            end
            
            
            % Parse operators
            if isnumeric(operators) && isscalar(operators)
                obj.OperatorNames = string.empty(1,0);
                obj.MaxOperators = uint64(operators);
                obj.mono_names = false;
            elseif isstring(operators)
                obj.OperatorNames = reshape(operators, 1, []);
                obj.MaxOperators = uint64(length(obj.OperatorNames));
                obj.mono_names = ~any(strlength(obj.OperatorNames)>1);
            elseif ischar(operators)
                obj.OperatorNames = string(operators(:))';
                obj.MaxOperators = uint64(length(obj.OperatorNames));
                obj.mono_names = true;
            else
                error("Argument 'operators' should either be the maximum"...
                      +" operator number, or a list of operator names.");
            end
            
            % Hermicity, interleave and normality
            obj.Hermitian = logical(is_hermitian);
            if is_hermitian && ~is_normal
                error("Hermitian operators must be normal.");
            end
            
            
            obj.Interleave = logical(interleave);
            obj.Normal = logical(is_normal);
            
            % Prepare 'extended names' map for formatting
            if ~isempty(obj.OperatorNames)
                if obj.Hermitian
                    obj.extended_names = obj.OperatorNames;
                elseif obj.Interleave 
                    conj_names = obj.OperatorNames + "*";
                    obj.extended_names = strings(1,2*length(obj.OperatorNames));
                    for i = 1:length(obj.OperatorNames)
                        obj.extended_names(2*i - [1, 0]) = ...
                            [obj.OperatorNames(i), conj_names(i)];
                    end
                else
                    obj.extended_names = [obj.OperatorNames, ...
                                          obj.OperatorNames + "*"];
                end
            end
               
            
            % Parse rules
            if isa(initialRules, 'Algebraic.OperatorRule')
                obj.Rules = reshape(initialRules, 1, []);
            elseif iscell(initialRules)
                obj.ImportCellArray(initialRules);
            else
                error(['Rules should be provided either as an array of',...
                       ' Algebraic.OperatorRule, or as a cell array.']);    
            end
            
        end
    end
    
    %% Add rules
    methods
        function AddRule(obj, new_rule, new_rule_rhs, negate, imaginary)
        % ADDRULE Add a generic rule to the rule book.
        % 
        % SYNTAX
        %   1. rb.AddRule(rule)
        %   2. rb.AddRule(lhs, rhs)
        %
        % PARAMS (Syntax 1)
        %   new_rule - An object of type Algebraic.OperatorRule, defining the rule
        %              or a cell array with two elements, the first
        %              defining the left-hand-side of the rule (pattern),
        %              the second the right-hand-side (replacement).
        %
        % PARAMS (Syntax 2)
        %   new_rule     - An array defining the operator string to match. 
        %   new_rule_rhs - An array defining the operator sequence to 
        %                  replace new_rule with.
        %   negate       - Set to true for LHS => -RHS.
		%   imaginary    - Set to true for LHS => i RHS (or -i RHS)
        %
        % See also: AlgebraicScenario, Algebraic.OperatorRule
        %
            
            % Complain if locked
            obj.errorIfLocked();
            
            if nargin < 4
                negate = false;
            else
                negate = logical(negate);
            end
			
			if nargin < 5
				imaginary = false;
			else
				imaginary = logical(imaginary)
			end
            
            % Construct rule from inputs and append to list
            if nargin >= 3
                obj.Rules(end+1) = Algebraic.OperatorRule(...
					new_rule, new_rule_rhs, negate, imaginary);
            elseif nargin >= 2
                if isa(new_rule, 'Algebraic.OperatorRule')
                    obj.Rules(end+1) = new_rule;
                elseif iscell(new_rule)
                    if isequal(size(new_rule), [1, 2])
                        obj.Rules(end+1) = Algebraic.OperatorRule(new_rule{1}, ...
                                                          new_rule{2});
                    elseif isequal(size(new_rule), [1, 3])
						
						if new_rule{2} == '-'
							negate = true;
							imaginary = false
						elseif new_rule{2} == 'i'
							negate = false;
							imaginary = true;
						elseif new_rule{2} == '-i'
							negate = true;
							imaginary = true;
						else
							error(obj.err_cellpair); % Can't be positive here
						end
					    obj.Rules(end+1) = Algebraic.OperatorRule(new_rule{1}, ...
                                                          new_rule{3}, ...
                                                          negate, imaginary);
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
        
        function AddCommutator(obj, lhs, rhs, anti_comm)
        % ADDCOMMUTATOR Add a rule imposing a commutation relationship
        % If input is (Y, X), the rule created is YX -> XY.
        % If anti-commutation flag is set, the rule created is YX -> -XY.
        %
        % PARAMS
        %   lhs - Left-hand side of the commutation bracket
        %   rhs - Right-hand side of the commutation bracket.
        %   anti_comm - Set to true to make an anti-commutator instead:
        %
        
            % Validate
            assert(nargin>=3);
            lhs = reshape(lhs, 1, []);
            rhs = reshape(rhs, 1, []);             
            if nargin<4
                anti_comm = false;
            end
      
            % Complain if locked
            obj.errorIfLocked();
            
            % Make and add rule
            rule_left = [lhs, rhs];
            rule_right = [rhs, lhs];            
            obj.AddRule(rule_left, rule_right, anti_comm);
      
        end
        
        function AddAntiCommutator(obj, lhs, rhs)
        % ADDCOMMUTATOR Add a rule imposing an anti-commutation relationship.
        % If input is (Y, X), the rule created is YX -> -XY.
        %
        % PARAMS
        %   lhs - Left-hand side of the anti-commutation bracket
        %   rhs - Right-hand side of the anti-commutation bracket.
        %
            
            obj.AddCommutator(lhs, rhs, true);
        end
        
                
        function MakeProjector(obj, symbol)
        % MAKEPROJECTOR Impose that an operator (or sequence) is projective.
        % If input is X, the rule created is XX -> X.
        %
        % PARAMS
        %   symbol - The operator, or operator sequence, X that is
        %   projective.
        %
            
            % Validate
            assert(nargin>=2);
            symbol = reshape(symbol, 1, []);
            
            % Make and add rule
            lhs = [symbol, symbol];
            obj.AddRule(lhs, symbol);
            
        end
        
                 
        function MakeHermitian(obj, symbol)
        % MAKEHERMITIAN Impose that an operator (or sequence) is Hermitian.
        % If input is X, the rule created is X* -> X.
        %
        % PARAMS
        %   symbol - The operator, or operator sequence, that is Hermitian.
        %
           
            % Validate
            assert(nargin>=2);
            symbol = reshape(symbol, 1, []);
            
            % Make and add rule
            lhs = obj.RawConjugate(symbol);
            obj.AddRule(lhs, symbol);            
        end
        
        function MakeUnitary(obj, symbol)
        % MAKEUNITARY Impose that an operator (or sequence) is unitary.
        % If input is X, the rules created are X* X -> I, and X X* -> I
        %
        % PARAMS
        %   symbol - The operator, or operator sequence, that is unitary.
        %

        
            % Validate
            assert(nargin>=2);
            symbol = reshape(symbol, 1, []);

            % Make and add rule
            conjugated = obj.RawConjugate(symbol);
            obj.AddRule([conjugated, symbol], []);
            obj.AddRule([symbol, conjugated], []);
        end
        
            
        function MakeSelfInverse(obj, symbol)
        % MAKEUNITARY Impose that an operator (or sequence) is self-inverse.
        % If input is X, the rule created is X X -> I.
        %
        % PARAMS
        %   symbol - The operator, or operator sequence, that is self-inverse.
        %
            
            % Validate
            assert(nargin>=2);
            symbol = reshape(symbol, 1, []);
            
            % Make and add rule
            obj.AddRule([symbol, symbol], []);
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
        
            % Validate
            if nargin < 2
                max_iterations = 20;
            else
                max_iterations = uint64(max_iterations);
            end
            
            if nargin < 3
                verbose = false;
            else
                verbose = logical(verbose);
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
                if obj.Interleave
                    extra_params{end+1} = 'interleaved';
                else
                    extra_params{end+1} = 'bunched';
                end
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
        % ISCOMPLETE Test if the set of rules is complete.
        %
            if ~obj.tested_complete
                params = {};
                if obj.Hermitian
                    params{end+1} = 'hermitian';
                else
                    if obj.Interleave
                        params{end+1} = 'interleaved';
                    else
                        params{end+1} = 'bunched';
                    end
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
        function ImportCellArray(obj, input)
        % IMPORTCELLARRAY Converts cell array into array of Algebraic.OperatorRule objects.
        %
        % PARAMS
        %   input - The cell array to parse.
        %
            
            % Validate
            if nargin < 2 || ~iscell(input)
                error("Must specify cell array input.");
            end
            
            obj.errorIfLocked();
            
            input = reshape(input, 1, []);
            obj.Rules = Algebraic.OperatorRule.empty(1,0);
            for index = 1:length(input)
                rule = input{index};
                if ~iscell(input)
                    error(obj.err_cellpair);
                end
                rhs_index = 2;				
                negate = false;
				imaginary = false;
                if isequal(size(rule), [1, 3])
                    if rule{2} == '-'
                        negate = true;
						imaginary = false;
                        rhs_index = 3;
                    elseif rule{2} == 'i'
						negate = false;
						imaginary = true;
						rhs_index = 3;
					elseif rule{2} == '-i'
						negate = true;
						imaginary = true;
						rhs_index = 3;
					else
                        error(obj.err_cellpair);
                    end
                elseif ~isequal(size(rule), [1, 2])
                    error(obj.err_cellpair);
                end
                lhs = rule{1};
                rhs = rule{rhs_index};
                                
                lhs = obj.ToStringArray(lhs);
                if isequal(rhs, uint64(0))
                    is_zero = true;
                else
                    is_zero = false;
                    rhs = obj.ToStringArray(rhs);
                end
                
                obj.Rules(end+1) = Algebraic.OperatorRule(lhs, rhs, negate, imaginary);
            end
            
            obj.tested_complete = false;
        end
        
        function val = ExportCellArray(obj)
        % EXPORTCELLARRAY Produces cell array describing rules of rulebook.
        %
        %
            val = cell(1, length(obj.Rules));
            for index = 1:length(obj.Rules)
                if obj.Rules(index).Negated || obj.Rules(index).Imaginary
                    val{index} = cell(1, 3); 
                    val{index}{1} = obj.Rules(index).LHS;
                    
					if obj.Rules(index).Imaginary
						if obj.Rules(index).Negated
							val{index}{2} = '-i';
						else
							val{index}{2} = 'i';
						end							
					else % must be negative real
						val{index}{2} = '-';
					end
                    val{index}{3} = obj.Rules(index).RHS;
                else
                    val{index} = cell(1, 2);
                    val{index}{1} = obj.Rules(index).LHS;
                    val{index}{2} = obj.Rules(index).RHS;
                end
            end
        end
    end
    
    %% Raw manipulation of operator strings
    methods
        function conj = RawConjugate(obj, op_str)
            
            assert(nargin==2)
            op_str = reshape(op_str, 1, []);
            
            % If char array, parse first to string
            is_char = ischar(op_str);
            if is_char
                if obj.mono_names
                    op_str = string(op_str(:))';
                else
                    op_str = string(op_str);
                end
            end
                        
            % Conjugate as string
            if isstring(op_str)
                conj = flip(op_str, 2);
                
                starred = ~endsWith(conj, "*");
                conj = strip(conj, "*");
                for i=1:length(conj)
                    if starred(i)
                        conj(i) = conj(i) + "*";
                    end
                end
                
                % Originally char, reconstruct char
                if is_char
                    conj = char(strjoin(conj,''));
                end
                return;
            end
            
            % Conjugate in terms of operator numbers
            if isnumeric(op_str)
                conj = uint64(flip(op_str, 2));
                if ~obj.Hermitian
                    if ~obj.Interleave
                        conj = conj + obj.MaxOperators - 1;
                        conj = mod(conj , obj.MaxOperators*2);
                        conj = conj + 1;                    
                    else
                        conj = conj - 1;
                        conj = bitxor(conj, uint64(1));
                        conj = conj + 1;
                    end
                end
                return;
            end
        end
        
        function str = ToStringArray(obj, input)
        % TOSTRING Format operator input sequence into string array.
        %
                
            assert(nargin==2)
            input = reshape(input, 1, []);
        
            % Do nothing if input is already string
            if isstring(input)
                str = input;
                return;
            end
            
            % Split up char array
            if ischar(input)
                str = string(input(:))';
                return
            end
            
            % Complain if not numeric now
            if ~isnumeric(input)
                error("Expected string array, character array, "...
                      +"or numeric array");
            end
                            
            % Default to 'X', if no op names
            if isempty(obj.OperatorNames)
                str = reshape("X" + input, 1, []);
                return;
            end
            
            % Do nothing if out of range
            if any(input <= 0) || any(input > length(obj.extended_names))
                str = input;
                return;
            end
           
            
            % Translate
            str = reshape(obj.extended_names(input), 1, []);
        end
    end
    
    %% Locking methods
    methods(Access={?Algebraic.OperatorRulebook, ?AlgebraicScenario})
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

