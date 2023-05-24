classdef MomentRuleBook < handle
%MOMENTRULEBOOK A list of substitutions to make at the level of moments.
%

    %% Properties
    properties(GetAccess = public, SetAccess = protected)
        Scenario % Associated scenario.
        RuleBookId % ID of the rulebook within the matrix system.
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
        end
    end
end
    