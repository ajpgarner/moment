classdef CompleteTest < MTKTestBase
    %COMPLETETEST Unit tests for complete mex function
    
    properties(Constant)
    end
    
    methods
        function check_completion_nh(testCase, ops, input, expected)
            output = mtk('complete', 'nonhermitian', ...
                         'operators', ops, input);
            testCase.verifyEqual(output, expected);
        end
        
        function check_completion_h(testCase, input, expected)
            output = mtk('complete', 'hermitian', input);
            testCase.verifyEqual(output, expected);
        end
    end
    
    methods (Test)
        function ABtoA_BAtoA(testCase)
            input = {{[1, 2], [1]}, {[2, 1], [2]}};
            expected = {{uint64([1, 1]), uint64([1])}, ...
                {uint64([1, 2]), uint64([1])}, ...
                {uint64([2, 1]), uint64([2])}, ...
                {uint64([2, 2]), uint64([2])}, ... 
                {uint64([3, 3]), uint64([3])}, ...
                {uint64([3, 4]), uint64([4])}, ...
                {uint64([4, 3]), uint64([3])}, ...
                {uint64([4, 4]), uint64([4])}};
            testCase.check_completion_nh(2, input, expected);
        end
        
        function AAAtoI_BBBtoI_ABABABtoI(testCase)
            input = {{[1, 1, 1], []}, {[2, 2, 2], []}, ...
                {[1, 2, 1, 2, 1, 2], []}};
            expected = {{uint64([1, 1, 1]), uint64.empty(1,0)}, ...
                {uint64([2, 2, 2]), uint64.empty(1,0)}, ...
                {uint64([3, 3, 3]), uint64.empty(1,0)}, ...
                {uint64([4, 4, 4]), uint64.empty(1,0)}, ...
                {uint64([2, 1, 2, 1]), uint64([1, 1, 2, 2])}, ...
                {uint64([2, 2, 1, 1]), uint64([1, 2, 1, 2])}, ...
                {uint64([4, 3, 4, 3]), uint64([3, 3, 4, 4])}, ...
                {uint64([4, 4, 3, 3]), uint64([3, 4, 3, 4])}};
            testCase.check_completion_nh(2, input, expected);
        end
        
        function Herm_ABtoA_BCtoB_CAtoA(testCase)
            input = {{[1, 2], [1]}, {[2, 3], [2]}, ...
                {[3, 1], [3]}};
            expected = {{uint64([2]), uint64([1])}, ...
                {uint64([3]), uint64([1])}, ...
                {uint64([1, 1]), uint64([1])}};
            testCase.check_completion_h(input, expected);
        end
        
        
        function NonHerm_InferHermitian(testCase)
            input = {{[1, 2], [1]}};
            expected = {{uint64([2]), uint64([1])}, ...
                {uint64([1, 1]), uint64([1])}};
            testCase.check_completion_nh(1, input, expected);
        end
                
        function Normal(testCase)
            input_rules = {};
            expected = {{uint64([3 1]), uint64([1 3])}, ...
                        {uint64([4 2]), uint64([2 4])}};
            output = mtk('complete', 'nonhermitian', 'normal', ...
                         'operators', 2, input_rules);
            testCase.verifyEqual(output, expected);            
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('complete');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_HermNoherm(testCase)
            function no_in()
                [~] = mtk('complete', 'hermitian', 'nonhermitian', ...
                            {{[1],[]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:mutex_param');
        end
        
        
        function Error_BadRule1(testCase)
            function no_in()
                ref_id = mtk('complete', 'operators', 1, ...
                    {{[1 1], [2]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule2(testCase)
            function no_in()
                ref_id = mtk('complete', ...
                    {{[1], [1 1]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule3(testCase)
            function no_in()
                ref_id = mtk('complete', ...
                    {{[1 1], [1 2]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule4(testCase)
            function no_in()
                ref_id = mtk('complete', ...
                    {{[1 1]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadRule5(testCase)
            function no_in()
                ref_id = mtk('complete', ...
                    {{[1 1], ["Not a number"]}});
            end
            testCase.verifyError(@() no_in(), 'mtk:could_not_convert');
        end
    end
    
end