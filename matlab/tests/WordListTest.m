classdef WordListTest < MTKTestBase
    %CONJUGATETEST Unit tests for conjugate function
    
    methods (Test)
        function Basic_Level0(testCase)
            ms_id = mtk('algebraic_matrix_system', 2);
            output = mtk('word_list', ms_id, 0);
            expected = {uint64.empty(1,0)};
            testCase.verifyEqual(output, expected);
        end
        
        function Basic_Level1(testCase)
            ms_id = mtk('algebraic_matrix_system', 2);
            output = mtk('word_list', ms_id,1);
            expected = {uint64.empty(1,0); ...
                        uint64([1]); ...
                        uint64([2])};
            testCase.verifyEqual(output, expected);
        end
        
        function Basic_Level2(testCase)
            ms_id = mtk('algebraic_matrix_system', 2);
            output = mtk('word_list', ms_id, 2);
            expected = {uint64.empty(1,0); ...
                        uint64([1]); ...
                        uint64([2]); ...
                        uint64([1 1]); ...
                        uint64([1 2]); ...
                        uint64([2 1]); ...
                        uint64([2 2])};
            testCase.verifyEqual(output, expected);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('word_list');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()
                ms_id = mtk('algebraic_matrix_system', 2);
                [~] = mtk('word_list', ms_id);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_BadMS(testCase)
            function no_in()
                ms_id = mtk('algebraic_matrix_system', 2);
                [~] = mtk('word_list', ms_id+1, 5);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
    end
end
