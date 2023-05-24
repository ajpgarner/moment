classdef CreateMomentRulesTest < MTKTestBase
    %APPLYVALUESTEST Unit tests for apply_values function
    methods (Test)
        function SubList_Empty(testCase)            
            ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            [rules_index, rules] = mtk('create_moment_rules', ref_id, {});
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));
            [rules_index2, rules2] = mtk('create_moment_rules', ref_id, {});
            testCase.verifyEqual(rules_index2, uint64(1));
            testCase.verifyTrue(isempty(rules2));
        end
        
        function SubList_Simple(testCase)            
            ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            [rules_index, rules] = mtk('create_moment_rules', ...
                                       ref_id, {{2, 0.5}});
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.5}}}});
        end
        
        function SubList_Multi(testCase)            
            ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
            mm_index = mtk('moment_matrix', ref_id, 1);
            [rules_index, rules] = mtk('create_moment_rules', ref_id, ...
                                       {{2, 0.3}, {3, 0.4}});
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.3}}}; ...
                 {uint64(3), {{uint64(1), 0.4}}}});
        end
    end
end
