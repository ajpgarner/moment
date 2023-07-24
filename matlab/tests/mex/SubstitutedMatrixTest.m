classdef SubstitutedMatrixTest < MTKTestBase
    %APPLYVALUESTEST Unit tests for substituted_matrix function
    methods (Test)
        function SimpleSubstitution(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {{2, 0.5}});
            sub_index = mtk('substituted_matrix', ref_id, ...
                             mm_index, rules_index);

            base_mm = mtk('operator_matrix', 'sequence_string', ref_id, mm_index);
            sub_mm = mtk('operator_matrix', 'sequence_string', ref_id, sub_index);
            base_mm(1,2) = "0.5";
            base_mm(2,2) = "0.5";
            base_mm(2,1) = "0.5";
            testCase.verifyEqual(sub_mm, base_mm);
   
            base_sm = mtk('operator_matrix', 'symbol_string', ref_id, mm_index);
            sub_sm = mtk('operator_matrix', 'symbol_string', ref_id, sub_index);
            base_sm(1,2) = "0.5";
            base_sm(2,2) = "0.5";
            base_sm(2,1) = "0.5";
            testCase.verifyEqual(sub_sm, base_sm);
        end
        
        function MultiSubstitution(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              {{2, 0.3}, {3, 0.4}});
            sub_index = mtk('substituted_matrix', ref_id, ...
                            mm_index, rules_index);
            
            base_mm = mtk('operator_matrix', 'sequence_string', ref_id, mm_index);
            sub_mm = mtk('operator_matrix', 'sequence_string', ref_id, sub_index);
            base_mm(1,2) = "0.3";
            base_mm(1,3) = "0.4";
            base_mm(2,2) = "0.3";
            base_mm(2,1) = "0.3";
            base_mm(3,1) = "0.4";
            base_mm(3,3) = "0.4";
            
            testCase.verifyEqual(sub_mm, base_mm);
   
            base_sm = mtk('operator_matrix', 'symbol_string', ref_id, mm_index);
            sub_sm = mtk('operator_matrix', 'symbol_string', ref_id, sub_index);
            base_sm(1,2) = "0.3";
            base_sm(1,3) = "0.4";
            base_sm(2,2) = "0.3";
            base_sm(2,1) = "0.3";
            base_sm(3,1) = "0.4";
            base_sm(3,3) = "0.4";
            testCase.verifyEqual(sub_sm, base_sm);
        end
        
        function ZeroSubstitution(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {{3, 0.0}});
            sub_index = mtk('substituted_matrix', ref_id, mm_index, rules_index);
            
            base_mm = mtk('operator_matrix', 'sequence_string', ref_id, mm_index);
            sub_mm = mtk('operator_matrix', 'sequence_string', ref_id, sub_index);
            base_mm(1,3) = "0";
            base_mm(3,3) = "0";
            base_mm(3,1) = "0";
            testCase.verifyEqual(sub_mm, base_mm);
            
            
            base_sm = mtk('operator_matrix', 'symbol_string', ref_id, mm_index);
            sub_sm = mtk('operator_matrix', 'symbol_string', ref_id, sub_index);
            base_sm(1,3) = "0";
            base_sm(3,3) = "0";
            base_sm(3,1) = "0";
            testCase.verifyEqual(sub_sm, base_sm);
        end     
        
        function FactoredSubstitution(testCase)            
            ref_id = mtk('inflation_matrix_system', [2, 2], {}, 1);
            mm_index = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {{2, 0.5}});
            sub_index = mtk('substituted_matrix', ref_id, ...
                            mm_index, rules_index);
           
            sub_mm = mtk('operator_matrix', 'sequence_string', ref_id, sub_index);
            expected_mm = [["1", "0.5", "<B>"]; ...
                           ["0.5", "0.5", "0.5 <B>"];...
                           ["<B>", "0.5 <B>", "<B>"]];
            testCase.verifyEqual(sub_mm, expected_mm);
                   
            sub_sm = mtk('operator_matrix', 'symbol_string', ref_id, sub_index);
            expected_sm = [["1", "0.5", "3"]; ...
                           ["0.5", "0.5", "0.5*3"];...
                           ["3", "0.5*3", "3"]];
            testCase.verifyEqual(sub_sm, expected_sm);
        end  
        
        function FactoredZeroSubstitution(testCase)            
            ref_id = mtk('inflation_matrix_system', [2, 2], {}, 1);
            mm_index = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {{2, 0}});
            sub_index = mtk('substituted_matrix', ref_id, ...
                            mm_index, rules_index);
           
            sub_mm = mtk('operator_matrix', 'sequence_string', ref_id, sub_index);
            expected_mm = [["1", "0", "<B>"]; ...
                           ["0", "0", "0"];...
                           ["<B>", "0", "<B>"]];
            testCase.verifyEqual(sub_mm, expected_mm);
                   
            sub_sm = mtk('operator_matrix', 'symbol_string', ref_id, sub_index);
            expected_sm = [["1", "0", "3"]; ...
                           ["0", "0", "0"];...
                           ["3", "0", "3"]];
            testCase.verifyEqual(sub_sm, expected_sm);
        end     
    end
end
