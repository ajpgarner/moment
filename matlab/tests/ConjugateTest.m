classdef ConjugateTest < MTKTestBase
    %CONJUGATETEST Unit tests for conjugate function
    
    methods (Test)
        function Identity(testCase)
            ms_id = mtk('new_algebraic_matrix_system', 2);
            input = uint64.empty(1,0);
            output = mtk('conjugate', ms_id, input);
            expected = uint64.empty(1,0);
            testCase.verifyEqual(output, expected);
        end
        
        function FundamentalOperator(testCase)
            ms_id = mtk('new_algebraic_matrix_system', 2);
            input = uint64([2]);
            output = mtk('conjugate', ms_id, input);
            expected = uint64([2]);
            testCase.verifyEqual(output, expected);
        end
        
        function SimpleString(testCase)
            ms_id = mtk('new_algebraic_matrix_system', 2);
            input = uint64([1, 2]);
            output = mtk('conjugate', ms_id, input);
            expected = uint64([2, 1]);
            testCase.verifyEqual(output, expected);
        end
        
        function NonHermitianSystem(testCase)
            ms_id = mtk('new_algebraic_matrix_system', 2, 'nonhermitian');
            input = uint64([1, 2]);
            output = mtk('conjugate', ms_id, input);
            expected = uint64([4, 3]);
            testCase.verifyEqual(output, expected);
        end
        
         function WithHash(testCase)
            ms_id = mtk('new_algebraic_matrix_system', 2, 'nonhermitian');
            input = uint64([1, 2]);
            [output, output_hash] = mtk('conjugate', ms_id, input);
            expected = uint64([4, 3]);
            testCase.verifyEqual(output, expected);
            testCase.verifyEqual(output_hash, ...
                                Util.shortlex_hash(4, expected));
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('conjugate');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()
                ms_id = mtk('new_algebraic_matrix_system', 2);
                [~] = mtk('conjugate', ms_id);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_BadMS(testCase)
            function no_in()
                ms_id = mtk('new_algebraic_matrix_system', 2);
                [~] = mtk('conjugate', ms_id+1, [1 2]);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
    end
end
