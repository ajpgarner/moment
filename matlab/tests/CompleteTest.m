classdef CompleteTest < NPATKTestBase
    %COMPLETETEST Unit tests for complete function
    
    properties(Constant)
    end
    
    methods
        function check_completion(testCase, input, expected)
            output = npatk('complete', input);
            testCase.verifyEqual(output, expected);
        end
    end
    
    methods (Test)
        function ABtoA_BAtoA(testCase)
            input = {{[0, 1], [0]}, {[1, 0], [1]}};
            expected = {{uint64([0, 0]), uint64([0])}, ...
                        {uint64([0, 1]), uint64([0])}, ...
                        {uint64([1, 0]), uint64([1])}, ...
                        {uint64([1, 1]), uint64([1])}};
            testCase.check_completion(input, expected);     
        end
        
         function AAAtoI_BBBtoI_ABABABtoI(testCase)
            input = {{[0, 0, 0], []}, {[1, 1, 1], []}, ...
                     {[0, 1, 0, 1, 0, 1], []}};
            expected = {{uint64([0, 0, 0]), uint64.empty(1,0)}, ...
                        {uint64([1, 1, 1]), uint64.empty(1,0)}, ...
                        {uint64([1, 0, 1, 0]), uint64([0, 0, 1, 1])}, ...
                        {uint64([1, 1, 0, 0]), uint64([0, 1, 0, 1])}};
            testCase.check_completion(input, expected);     
        end
    end
end