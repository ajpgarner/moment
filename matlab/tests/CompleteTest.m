classdef CompleteTest < NPATKTestBase
    %COMPLETETEST Unit tests for complete function
    
    properties(Constant)
    end
    
    methods
        function check_completion_nh(testCase, input, expected)
            output = npatk('complete', 'nonhermitian', input);
            testCase.verifyEqual(output, expected);
        end
        
        function check_completion_h(testCase, input, expected)
            output = npatk('complete', 'hermitian', input);
            testCase.verifyEqual(output, expected);
        end
    end
    
    methods (Test)
        function ABtoA_BAtoA(testCase)
            input = {{[1, 2], [1]}, {[2, 1], [2]}};
            expected = {{uint64([1, 1]), uint64([1])}, ...
                {uint64([1, 2]), uint64([1])}, ...
                {uint64([2, 1]), uint64([2])}, ...
                {uint64([2, 2]), uint64([2])}};
            testCase.check_completion_nh(input, expected);
        end
        
        function AAAtoI_BBBtoI_ABABABtoI(testCase)
            input = {{[1, 1, 1], []}, {[2, 2, 2], []}, ...
                {[1, 2, 1, 2, 1, 2], []}};
            expected = {{uint64([1, 1, 1]), uint64.empty(1,0)}, ...
                {uint64([2, 2, 2]), uint64.empty(1,0)}, ...
                {uint64([2, 1, 2, 1]), uint64([1, 1, 2, 2])}, ...
                {uint64([2, 2, 1, 1]), uint64([1, 2, 1, 2])}};
            testCase.check_completion_nh(input, expected);
        end
        
        function Herm_ABtoA_BCtoB_CAtoA(testCase)
            input = {{[1, 2], [1]}, {[2, 3], [2]}, ...
                {[3, 1], [3]}};
            expected = {{uint64([2]), uint64([1])}, ...
                {uint64([3]), uint64([1])}, ...
                {uint64([1, 1]), uint64([1])}};
            testCase.check_completion_h(input, expected);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = npatk('complete');
            end
            testCase.verifyError(@() no_in(), 'npatk:too_few_inputs');
        end
        
        function Error_HermNoherm(testCase)
            function no_in()
                [~] = npatk('complete', 'hermitian', 'nonhermitian', ...
                            {{[1],[]}});
            end
            testCase.verifyError(@() no_in(), 'npatk:mutex_param');
        end
    end
    
end