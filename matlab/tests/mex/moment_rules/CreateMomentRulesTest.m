classdef CreateMomentRulesTest < MTKTestBase
%CREATEMOMENTRULESTEST Unit tests for create_moment_rules mex function
    
    methods (Test, TestTags={'mex'})
        function SubList_Empty(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));
            rules_index2 = mtk('create_moment_rules', ref_id, {});
            testCase.verifyEqual(rules_index2, uint64(1));
        end
        
        function SubList_Simple(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'list', ...
                             ref_id, {{2, 0.5}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.5}, {int64(2), -1}}});
        end
        
        function SubList_Multi(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'list', ...
                               ref_id, {{2, 0.3}, {3, 0.4}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.3}, {int64(2), -1.0}}; ...
                 {{int64(1), 0.4}, {int64(3), -1.0}}});
        end
        
          function Symbols_Empty(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id,...
                              'input', 'symbols', {});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));            
        end
        
        function Symbols_Simple(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',...
                              ref_id, {{{2, -1.0}, {1, 0.5}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.5}, {int64(2), -1.0}}});
        end
        
        function Symbols_Multi(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',... 
                               ref_id, ...
                               {{{2, -1.0}, {1, 0.3}};...
                                {{3, -1.0}, {1, 0.4}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.3}, {int64(2), -1.0}}; ...
                 {{int64(1), 0.4}, {int64(3), -1.0}}});
        end
                
        function Symbols_Chain(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',...
                              ref_id, ...
                              {{{2, -1.0}, {1, 0.3}}; ...
                               {{3, -1.0}, {2, 0.4}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.3}, {int64(2), -1.0}}; ...
                 {{int64(1), 0.12}, {int64(3), -1.0}}});
        end
        
        function OpSeq_Existing_Empty(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                             'input', 'sequences', 'no_new_symbols', {});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));   
        end
        
         function OpSeq_Existing_Simple(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', 'no_new_symbols', ...
                              {{{[2], -1.0}, {[1], 0.5}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(2), 0.5}, {int64(3), -1.0}}}); % Rule is A1 -> 0.5 A0
         end
        
         
        function OpSeq_Existing_Chain(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', 'no_new_symbols', ...
                              {{{[1], -1.0}, {[], 0.3}}; ...
                               {{[2], -1.0}, {[1], 0.4}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.3}, {int64(2), -1.0}}; ...
                 {{int64(1), 0.12}, {int64(3), -1.0}}});
        end

        function OpSeq_NewSymbols_Empty(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            old_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(old_symbols), 2);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', {});                    
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            new_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(new_symbols), 2);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));   
        end
        
         function OpSeq_NewSymbols_Simple(testCase)            
            ref_id = mtk('algebraic_matrix_system', 2, 'tolerance', 10);
            old_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(old_symbols), 2);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', ...
                              {{{[2], -1.0}, {[1], 0.5}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
                   
            new_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(new_symbols), 4);
            
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(3), 0.5}, {int64(2), -1.0}}}); % Rule is Y -> 0.5 X
            
         end
        
         
        function OpSeq_NewSymbols_Chain(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            old_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(old_symbols), 2);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', ...
                              {{{[1], -1.0}, {[], 0.3}}; ...
                               {{[2], -1.0}, {[1], 0.4}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.3}, {int64(2), -1.0}}; ...
                 {{int64(1), 0.12}, {int64(3), -1.0}}});
            new_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(new_symbols), 4);
        end
        
        function AppendRules_NoClash(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {{2, 0.3}});
            second_index = mtk('create_moment_rules', ref_id, ...
                               'rulebook', rules_index, {{3, 0.4}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(second_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.3}, {int64(2), -1.0}}; ...
                 {{int64(1), 0.4}, {int64(3), -1.0}}});
        end
                 
        function WithLabel(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',...
                              'label', "Named rulebook", ...
                               ref_id, {{{2, -1.0}, {1, 0.5}}});
            rules = mtk('moment_rules', ref_id, rules_index, 'symbols');
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{{int64(1), 0.5}, {int64(2), -1.0}}});
        end
        
    end
    
   
    methods (Test, TestTags={'Mex', 'Error'})
        function Error_NoInput(testCase)
            function no_in()
                [~] = mtk('create_moment_rules');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooManyInputs(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, ref_id, ref_id);
            end
            testCase.verifyError(@() bad_call(), 'mtk:too_many_inputs');
        end
        
        function Error_BadMatrixSystem(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id+1, {});
            end
            testCase.verifyError(@() bad_call(), 'mtk:storage_error');
        end
        
        function Error_SymbolNotFound_SubList(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, {{100, 1.0}});
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
        
        function Error_SymbolNotFound_SymbolPolynomial(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, 'input', 'symbols', ...
                             {{{100, 1.0}}});
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
        
        function Error_SymbolNotFound_SequenceNoCreate(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, ...
                             'input', 'sequences', 'no_new_symbols', ...
                             {{{[1 2], 1.0}}});
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
    end
end
