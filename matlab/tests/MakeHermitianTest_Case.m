classdef MakeHermitianTest_Case
    %MAKESYMMETRICTEST_CASE Subclass, for each example of make_symmetric
    
    properties
        input_string;
        expected_string;
        expected_subs;
    end
      
    methods
        function obj = MakeHermitianTest_Case(str_input, str_expect, ...
                                              raw_subs)            
            obj.input_string = str_input;
            obj.expected_string = str_expect;
            obj.expected_subs = raw_subs;
        end
            
        function StringToString(testCase, testObj)
           [actual_string, actual_subs] = npatk('make_hermitian', ...
                                                testCase.input_string);
           testObj.verifyEqual(actual_string, testCase.expected_string);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
        end
    end
end