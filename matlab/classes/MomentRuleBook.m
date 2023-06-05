classdef MomentRuleBook < handle
%MOMENTRULEBOOK A list of substitutions to make at the level of moments.
%

%% Properties
properties(GetAccess = public, SetAccess = protected)
    Scenario    % Associated scenario.
    RuleBookId  % ID of the rulebook within the matrix system.
    RawRules    % Rules as cell array of symbol substitutions.
    RuleStrings % Rules as strings
    
    % Multiplier of epsilon, before coefficients are treated as zero.
    ZeroTolerance
end


properties(Constant, Access = private)
    err_locked = ['No more changes to this ruleset are possible, ',...
                  'because it has already been applied to a matrix.'];
end
    

%% Constructor
methods
    function obj = MomentRuleBook(scenario, label, tolerance)
    % MOMENTRULEBOOK Creates a moment substitution rule book.
        arguments
            scenario (1,1) Abstract.Scenario
            label (1,1) string = ""
            tolerance (1,1) double {mustBeNonnegative} = 1.0 
        end    
        obj.Scenario = scenario;
        
        % Extra arguments to MTK
        rb_args = cell(1,0);
        if tolerance ~= 1.0 
            obj.ZeroTolerance = double(tolerance);
            rb_args{end+1} = "tolerance";
            rb_args{end+1} = double(tolerance);
        else
            obj.ZeroTolerance = 1.0;
        end
        
        if ~isempty(label) && (label ~= "")
            rb_args{end+1} = "label";
            rb_args{end+1} = label;
        end
                   
        obj.RuleBookId = mtk('create_moment_rules', rb_args{:}, ...
                             scenario.System.RefId, {});
                         
        obj.RawRules = cell.empty(0,1);
        obj.RuleStrings = string.empty(0,1);
    end
end

%% Add rules

%% Static 'constructors'
methods
    function AddScalarSubstitution(obj, symbol_ids, values)
    % SCALARSUBSTITUTION Creates a simple substitution rulebook.
    %
    % Each symbol id is replaced with the corresponding value.
    %
    % Symbols and values are provided as two arrays of equal length.
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

    function obj = AddScalarSubstitutionCell(obj, symbol_value_pairs)
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
        rb_args = {'list', 'rulebook', obj.RuleBookId};
        if obj.ZeroTolerance ~= 1.0 
            rb_args{end+1} = "tolerance";
            rb_args{end+1} = double(obj.ZeroTolerance);
        end
       
        % Import rules into rulebook
        [rule_id, rules, strs] = mtk('create_moment_rules', rb_args{:}, ...
                                      obj.Scenario.System.RefId, ...
                                      symbol_value_pairs);                               
        obj.RuleBookId = rule_id;
        obj.RawRules = rules;
        obj.RuleStrings = strs;
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
        extra_params = {'sequences'};
        if ~new_symbols
            extra_params{end+1} = 'no_new_symbols';
        end
        if obj.ZeroTolerance ~= 1.0 
            extra_params{end+1} = "tolerance";
            extra_params{end+1} = double(obj.ZeroTolerance);
        end

        % Construct rulebook, and import rules
        [rule_id, rules, strs] = mtk('create_moment_rules', extra_params{:},...
                                     obj.Scenario.System.RefId, ...                               
                                     'rulebook', obj.RuleBookId, ...
                                     op_seq_cell);                               
        obj.RuleBookId = rule_id;
        obj.RawRules = rules;
        obj.RuleStrings = strs;
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
    