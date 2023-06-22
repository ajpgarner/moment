classdef MomentRuleBook < handle
%MOMENTRULEBOOK A list of substitutions to make at the level of moments.
%

%% Properties
properties(GetAccess = public, SetAccess = protected)
    Scenario    % Associated scenario.
    RuleBookId  % ID of the rulebook within the matrix system.
end

properties(Dependent, GetAccess = public, SetAccess = private)
    RawRuleCell     % Rules as cell array of symbol substitutions.
    RulePolynomials % Rules as polynomials.
    RuleStrings     % Rules as strings    
end

properties(Access=private)
    cache_RawRuleCell;
    cache_RulePolynomials;
    cache_RuleStrings;
    has_rrc = false;
    has_rp = false;
    has_rs = false;
end

properties(Constant, Access = private)
    err_locked = ['No more changes to this ruleset are possible, ',...
                  'because it has already been applied to a matrix.'];
end
    

%% Constructor
methods
    function obj = MomentRuleBook(scenario, label)
    % MOMENTRULEBOOK Creates a moment substitution rule book.
        arguments
            scenario (1,1) Abstract.Scenario
            label (1,1) string = ""
        end    
        obj.Scenario = scenario;

        if nargin == 1
            label = "";
        end

        % Extra arguments to MTK
        rb_args = cell(1,0);       
        if ~isempty(label) && (label ~= "")
            rb_args{end+1} = "label";
            rb_args{end+1} = label;
        end
                   
        obj.RuleBookId = mtk('create_moment_rules', rb_args{:}, ...
                             scenario.System.RefId, {});
        
        obj.invalidate_cached_rules();        
    end
end

%% Getter methods
methods
    function val = get.RawRuleCell(obj)
        if ~obj.has_rrc
            obj.cache_RawRuleCell = ...
                mtk('moment_rules', obj.Scenario.System.RefId, ...
                    obj.RuleBookId, 'sequences');            
            obj.has_rrc = true;
        end
        val = obj.cache_RawRuleCell;
    end
    
    function val = get.RulePolynomials(obj)
        if ~obj.has_rp
            rrc = obj.RawRuleCell;
            obj.cache_RulePolynomials = Algebraic.Polynomial.empty(0, 1);
            for idx=1:length(rrc)
                obj.cache_RulePolynomials(end+1) = ...
                    Algebraic.Polynomial(obj.Scenario, rrc{idx});
            end
            obj.has_rp = true;
        end
        val = obj.cache_RulePolynomials;
    end
    
    function val = get.RuleStrings(obj)
        if ~obj.has_rs
            obj.cache_RuleStrings = ...
                mtk('moment_rules', obj.Scenario.System.RefId, ...
                    obj.RuleBookId, 'strings');
            obj.has_rs = true;
        end
        val = obj.cache_RuleStrings;
    end
    
    function invalidate_cached_rules(obj)
         obj.cache_RawRuleCell = cell.empty(0,1);
         obj.cache_RulePolynomials = Algebraic.Polynomial.empty(0, 1);
         obj.cache_RuleStrings = string.empty(0,1);
         obj.has_rrc = false;
         obj.has_rp = false;
         obj.has_rs = false;
    end
end


%% Add rules
methods
    function Add(obj, polynomials, new_symbols)
    % ADD Adds rules based on (list of) polynomials that should be zero.
    %
    % PARAMS
    %   polynomials - One or more polynomial rules.
    %   new_symbols - Set to true to create symbols in setting if they do
    %                 not already exist.
    %
    arguments
        obj (1,1) MomentRuleBook
        polynomials (:,1) Algebraic.Polynomial
        new_symbols (1,1) logical = true
    end
        raw_rules = cell(length(polynomials), 1);
        for idx=1:length(polynomials)
            raw_rules{idx} = polynomials(idx).OperatorCell;
        end
        obj.AddFromOperatorSequences(raw_rules, new_symbols);
    end
    
    function AddScalarSubstitution(obj, symbol_ids, values)
    % ADDSCALARSUBSTITUTION Add replacement rules, each symbol by a scalar.
    %
    % PARAMS
    %   symbol_ids - Array of symbol IDs to create rules for.
    %   values     - The corresponding value to assign to each symbol id.
    %
        arguments
            obj (1,1) MomentRuleBook
            symbol_ids (1,:) uint64
            values (1,:) double
        end

        % Check inputs, then join into a cell
        if length(symbol_ids) ~= length(values)
            error("Number of symbol IDs should match number of values.");
        end
        cell_input = cell(length(symbol_ids), 1);
        for idx=1:length(symbol_ids)
            cell_input{idx} = {uint64(symbol_ids(idx)), ...
                               double(values(idx))};
        end
        
        obj.AddScalarSubstitutionCell(cell_input);
    end

    function AddScalarSubstitutionCell(obj, symbol_value_pairs)
    % SCALARSUBSTITUTIONCELL Creates a simple substitution rulebook.
    % Each symbol id is replaced with the corresponding value.
    %
    % Symbols and values are provided as a single cell array of 
    % {symbol, value} cell pairs.
    %
        arguments
            obj (1,1) MomentRuleBook
            symbol_value_pairs (1,:) cell
        end

        % Extra arguments
        rb_args = {'input', 'list', 'rulebook', obj.RuleBookId};

       
        % Import rules into rulebook
        rule_id = mtk('create_moment_rules', rb_args{:}, ...
                      obj.Scenario.System.RefId, ...
                      symbol_value_pairs);
        assert(rule_id == obj.RuleBookId);
        obj.invalidate_cached_rules();
    end


    function AddFromOperatorSequences(obj, op_seq_cell, new_symbols)
    % FROMOPERATORSEQUENCES Creates from operator sequence cell array.
    %
    % Provide a cell array of polynomials; each polynomial represented as a
    % cell array of cell-array pairs {[operators], factor}.
    %
    % If new_symbols is true, then register any 'missing' operator
    % sequences. Otherwise, an error will be thrown if an unrecognised
    % sequence is supplied.
    %
        arguments
            obj (1,1) MomentRuleBook
            op_seq_cell (1,:) cell
            new_symbols (1,1) logical = false
        end
        
        % Prepare params
        extra_params = {'input', 'sequences', 'rulebook', obj.RuleBookId};
        if ~new_symbols
            extra_params{end+1} = 'no_new_symbols';
        end

        % Construct rulebook, and import rules
        rule_id = mtk('create_moment_rules', extra_params{:},...
                      obj.Scenario.System.RefId, op_seq_cell);
        assert(rule_id == obj.RuleBookId);
        obj.invalidate_cached_rules();
        
        % Flag to system object that extra symbols may have been created.
        if new_symbols
            obj.Scenario.System.UpdateSymbolTable();
        end
        
    end
end

%% Apply rules to objects
methods
    function val = Apply(obj, target)
        arguments
            obj (1,1) MomentRuleBook
            target
        end

        if isa(target, 'OpMatrix.OperatorMatrix')
            val = target.ApplyRules(obj);
        elseif isa(target, 'OpMatrix.CompositeOperatorMatrix')
            val = target.ApplyRules(obj);            
        elseif isa(target, 'Algebraic.Zero')
            val = target.ApplyRules(obj);
        elseif isa(target, 'Algebraic.Monomial')
            val = target.ApplyRules(obj);
        elseif isa(target, 'Algebraic.Polynomial')
            val = target.ApplyRules(obj);
        else
            error("Could not apply rules to target of type %s.", ...
                  class(target));
        end            
    end
end
    
end
    
