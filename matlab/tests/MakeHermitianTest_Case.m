classdef MakeHermitianTest_Case
    %MAKESYMMETRICTEST_CASE Subclass, for each example of make_symmetric
    
    properties
        input_string;
        expected_string;
        expected_subs;
        expected_sym;
    end
      
    methods
        function obj = MakeHermitianTest_Case(str_input, str_expect, ...
                                              raw_subs, raw_sym)            
            obj.input_string = str_input;
            obj.expected_string = str_expect;
            obj.expected_subs = raw_subs;
            obj.expected_sym = raw_sym;
        end
            
        function StringToString(testCase, testObj)
           [actual_string, actual_subs, actual_sym] = ...
                    npatk('make_hermitian', testCase.input_string);
           testObj.verifyEqual(actual_string, testCase.expected_string);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
           testObj.verifyEqual(actual_sym, testCase.expected_sym);
        end
    end
end