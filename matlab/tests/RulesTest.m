classdef RulesTest < MTKTestBase
    %RULESTESTS Unit tests for rules mex function
    methods (Test)
        function EmptyRules(testCase)
            expected = cell.empty(1,0);
            ref_id = mtk('new_algebraic_matrix_system', 2);
            comp_rules = mtk('rules', ref_id);
            testCase.verifyEqual(comp_rules, expected);
        end
        
        function IncompleteRules(testCase)
            raw_rules = {{[1, 1, 1], []}, {[2, 2, 2], []}, ...
                {[1, 2, 1, 2, 1, 2], []}};
            expected = {{uint64([1, 1, 1]), uint64.empty(1,0)}, ...
                {uint64([2, 2, 2]), uint64.empty(1,0)}, ...
                {uint64([1, 2, 1, 2, 1, 2]), uint64.empty(1,0)}};
            ref_id = mtk('new_algebraic_matrix_system', 'quiet',...
                         2, raw_rules);
            comp_rules = mtk('rules', ref_id);
            testCase.verifyEqual(comp_rules, expected);
        end
        
        function CompletedRules(testCase)
            raw_rules = {{[1, 1, 1], []}, {[2, 2, 2], []}, ...
                {[1, 2, 1, 2, 1, 2], []}};
            ref_id = mtk('new_algebraic_matrix_system', ...
                'complete_attempts', 20, ...
                'nonhermitian', 2, raw_rules);
            comp_rules = mtk('rules', ref_id);
            expected = {{uint64([1, 1, 1]), uint64.empty(1,0)}, ...
                {uint64([2, 2, 2]), uint64.empty(1,0)}, ...
                {uint64([2, 1, 2, 1]), uint64([1, 1, 2, 2])}, ...
                {uint64([2, 2, 1, 1]), uint64([1, 2, 1, 2])}};
            testCase.verifyEqual(comp_rules, expected);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('rules');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_BadRefID(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', 2);
                [~] = mtk('rules', ref_id+1);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_WrongSystem(testCase)
            function no_in()
                ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
                [~] = mtk('rules', ref_id);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
    end
end