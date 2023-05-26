classdef MomentRuleBook < handle
%MOMENTRULEBOOK A list of substitutions to make at the level of moments.
%

%% Properties
properties(GetAccess = public, SetAccess = protected)
    Scenario % Associated scenario.
    RuleBookId % ID of the rulebook within the matrix system.
    RawRules % Rules as cell array of symbol substitutions.
end

%% Constructor
methods
    function obj = MomentRuleBook(scenario)
    % MOMENTRULEBOOK Creates a moment substitution rule book.
        arguments
            scenario (1,1) Abstract.Scenario
        end    
        obj.Scenario = scenario;           
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
            val = obj.ApplyToMatrix(target);
        elseif isa(target, 'OpMatrix.CompositeOperatorMatrix')
            val = target.ApplyRules(obj);
        else
            error("Could not apply rules to target of type %s.", ...
                  class(target));
        end            
    end

    function val = ApplyToMatrix(obj, target)
        arguments
            obj (1,1) MomentRuleBook
            target (1,1) OpMatrix.OperatorMatrix
        end            
        val = target.ApplyRules(obj);           
    end
end

%% Static 'constructors'
methods(Static)
    function obj = ScalarSubstitution(scenario, symbol_ids, values)
    % SCALARSUBSTITUTION Creates a simple substitution rulebook.
    %
    % Each symbol id is replaced with the corresponding value.
    %
    % Symbols and values are provided as two arrays of equal length.
        arguments
            scenario (1,1) Abstract.Scenario
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

        % Construct rulebook, and import rules
        obj = MomentRuleBook(scenario);
        [rule_id, rules] = mtk('create_moment_rules', 'list', ...
                               scenario.System.RefId, ...
                               cell_input);                               
        obj.RuleBookId = rule_id;
        obj.RawRules = rules;
    end

    function obj = ScalarSubstitutionCell(scenario, symbol_value_pairs)
    % SCALARSUBSTITUTIONCELL Creates a simple substitution rulebook.
    % Each symbol id is replaced with the corresponding value.
    %
    % Symbols and values are provided as a single cell array of 
    % {symbol, value} cell pairs.
    %
        arguments
            scenario (1,1) Abstract.Scenario
            symbol_value_pairs (1,:) cell
        end

        % Construct rulebook, and import rules
        obj = MomentRuleBook(scenario);
        [rule_id, rules] = mtk('create_moment_rules', 'list', ...
                               scenario.System.RefId, ...
                               symbol_value_pairs);                               
        obj.RuleBookId = rule_id;
        obj.RawRules = rules;
    end


    function obj = FromOperatorSequences(scenario, op_seq_cell, new_symbols)
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
            scenario (1,1) Abstract.Scenario
            op_seq_cell (1,:) cell
            new_symbols (1,1) logical = false
        end
        
        % Prepare params
        extra_params = {'sequences'};
        if ~new_symbols
            extra_params{end+1} = 'no_new_symbols';
        end

        % Construct rulebook, and import rules
        obj = MomentRuleBook(scenario);
        [rule_id, rules] = mtk('create_moment_rules', extra_params{:},...
                               scenario.System.RefId, ...
                               op_seq_cell);                               
        obj.RuleBookId = rule_id;
        obj.RawRules = rules;
    end
end
end
    