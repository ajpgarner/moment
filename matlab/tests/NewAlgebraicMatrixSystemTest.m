classdef NewAlgebraicMatrixSystemTest < MTKTestBase
    %NEWALGEBRAICMATRIXSYSTEMTESTS Unit tests for new_algebraic_matrix_system
    % mex function
    methods (Test)
        function Basic(testCase)
            ref_id = mtk('new_algebraic_matrix_system', 2);
            testCase.verifyGreaterThan(ref_id, 0);
            mm = mtk('moment_matrix', ref_id, 1);
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(1));
            testCase.verifyEqual(sys_info.Symbols, uint64(7));
        end
        
        function SubRule(testCase)
            ref_id = mtk('new_algebraic_matrix_system', 1, {{[1 1], [1]}});
            testCase.verifyGreaterThan(ref_id, 0);
            mm = mtk('moment_matrix', ref_id, 1);
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(1));
            testCase.verifyEqual(sys_info.Symbols, uint64(3)); %0/1/X
        end
        
        function WithCompletion(testCase)
            raw_rules = {{[1, 1, 1], []}, {[2, 2, 2], []}, ...
                         {[1, 2, 1, 2, 1, 2], []}};
            ref_id = mtk('new_algebraic_matrix_system', ...
                         'complete_attempts', 20, ...
                         'nonhermitian', 2, raw_rules);
            comp_rules = mtk('rules', ref_id);
            expected = {{uint64([1, 1, 1]), uint64.empty(1,0)}, ...
                {uint64([2, 2, 2]), uint64.empty(1,0)}, ...
                {uint64([3, 3, 3]), uint64.empty(1,0)}, ...
                {uint64([4, 4, 4]), uint64.empty(1,0)}, ...
                {uint64([2, 1, 2, 1]), uint64([1, 1, 2, 2])}, ...
                {uint64([2, 2, 1, 1]), uint64([1, 2, 1, 2])}, ...
                {uint64([4, 3, 4, 3]), uint64([3, 3, 4, 4])}, ...
                {uint64([4, 4, 3, 3]), uint64([3, 4, 3, 4])}};
            
            testCase.verifyEqual(comp_rules, expected);
        end
        
        function NonHermitianButNormal(testCase)
            raw_rules = {};
            ref_id = mtk('new_algebraic_matrix_system', ...
                         'complete_attempts', 20, ...
                         'nonhermitian', 'normal', 2, raw_rules);
            comp_rules = mtk('rules', ref_id);
            expected = {{uint64([3, 1]), uint64([1, 3])}, ...
                        {uint64([4, 2]), uint64([2, 4])}};
            testCase.verifyEqual(comp_rules, expected);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooManyInputs(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    2, {{[1 1], 1}}, 2);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_many_inputs');
        end
        
        function Error_Contradiction(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    2, {{[1 1], 1}}, 'hermitian', 'nonhermitian');
            end
            testCase.verifyError(@() no_in(), 'mtk:mutex_param');
        end
        
        function Error_BadOpCount1(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    -1, {{[1 1], 1}});
            end
            testCase.verifyError(@() no_in(), 'mtk:negative_value');
        end
        
        function Error_BadOpCount2(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    "Unreadable", {{[1 1], 1}});
            end
            testCase.verifyError(@() no_in(), 'mtk:could_not_convert');
        end
        
        function Error_BadRule1(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    1, {{[1 1], [2]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule2(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    1, {{[1], [1 1]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule3(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    2, {{[1 1], [1 2]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule4(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    2, {{[1 1]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule5(testCase)
            function no_in()
                ref_id = mtk('new_algebraic_matrix_system', ...
                    2, {{[1 1], ["Not a number"]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:could_not_convert');
        end
    end
end