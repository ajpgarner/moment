classdef ApplyMomentRulesTest < MTKTestBase
%APPLYMOMENTRULESTEST Unit tests for `apply_moment_rules` mex function
    methods (Test)        
        function DoSubstitution(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            op_rules = {{{[1, 2], 1.0}, {[], -0.5}}}; % A0, B0 = 0.5
            rules_index = mtk('create_moment_rules', ...
                            'input', 'sequences', ... 
                            ref_id, op_rules);
            test_out = mtk('apply_moment_rules', ref_id, rules_index, ...
                           {{6, 0.5}, {1, 2.0}});
            expected_out = {{uint64(1), 2.25}};
            testCase.verifyEqual(test_out, expected_out);
        end
        
        function SequenceOutput(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            op_rules = {{{[1, 2], 1.0}, {[], -0.5}}}; % A0, B0 = 0.5
            rules_index = mtk('create_moment_rules', ...
                            'input', 'sequences', ... 
                            ref_id, op_rules);
            test_out = mtk('apply_moment_rules', 'output', 'polynomials',...
                           ref_id, rules_index, ...
                           {{6, 0.5}, {2, 10.0}, {1, 2.0}});           
            expected_out = {{uint64.empty(1,0); uint64(1)}, ...
                            complex([2.25; 10.0]), ...
                            uint64([1; 2]), ...
                            int64([1; 2]), ...
                            logical([false; false]), ...
                            int64([1; 2]), ...
                            int64([0; 0])};
            testCase.verifyEqual(test_out, expected_out);
        end
    end    
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()             
               [~] = mtk('apply_moment_rules');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');           
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()             
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('moment_matrix', ref_id, 1);
                op_rules = {{{[1, 2], 1.0}, {[], -0.5}}}; % A0, B0 = 0.5
                rules_index = mtk('create_moment_rules', ...
                            'input', 'sequences', ... 
                            ref_id, op_rules);
               [~] = mtk('apply_moment_rules', ref_id, rules_index);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');           
        end
       
        function Error_BadMS(testCase)
            function no_in()             
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('moment_matrix', ref_id, 1);
                op_rules = {{{[1, 2], 1.0}, {[], -0.5}}}; % A0, B0 = 0.5
                rules_index = mtk('create_moment_rules', ...
                            'input', 'sequences', ... 
                            ref_id, op_rules);
               [~] = mtk('apply_moment_rules', ref_id+1, rules_index, {});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');           
        end
   
        function Error_BadRules(testCase)
            function no_in()             
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('moment_matrix', ref_id, 1);
                op_rules = {{{[1, 2], 1.0}, {[], -0.5}}}; % A0, B0 = 0.5
                rules_index = mtk('create_moment_rules', ...
                            'input', 'sequences', ... 
                            ref_id, op_rules);
               [~] = mtk('apply_moment_rules', ref_id, rules_index+1, {});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');           
        end
      
        function Error_BadSymbols1(testCase)
            function no_in()             
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('moment_matrix', ref_id, 1);
                op_rules = {{{[1, 2], 1.0}, {[], -0.5}}}; % A0, B0 = 0.5
                rules_index = mtk('create_moment_rules', ...
                            'input', 'sequences', ... 
                            ref_id, op_rules);
               [~] = mtk('apply_moment_rules', ref_id, rules_index, "cake");
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');           
        end
        
        function Error_BadSymbols2(testCase)
            function no_in()             
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('moment_matrix', ref_id, 1);
                op_rules = {{{[1, 2], 1.0}, {[], -0.5}}}; % A0, B0 = 0.5
                rules_index = mtk('create_moment_rules', ...
                            'input', 'sequences', ... 
                            ref_id, op_rules);
               [~] = mtk('apply_moment_rules', ref_id, rules_index, ...
                            {{100, 1.0}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');           
        end   
    end
end