classdef VersionTest < matlab.unittest.TestCase
    %GENERATEBASISTEST Unit tests for version function
    
    properties(Constant)
        expected_string = '0.1.0';
        expected_struct = struct('major', int64(0), ...
                                 'minor', int64(1), ...
                                 'build', int64(0));
    end
       
    methods(TestMethodSetup)
        function addNPATKpath(testCase)
            import matlab.unittest.fixtures.PathFixture
            testCase.applyFixture(PathFixture([".."]));
        end
    end
    
    methods(TestMethodTeardown)
        function clearNPATK(testCase)
            clear npatk
        end
    end
    
    methods (Test)
        function version_string(testCase)
            actual_string = npatk('version');
            testCase.verifyEqual(actual_string, testCase.expected_string);
        end
        
        function version_struct(testCase)
            actual_struct = npatk('version', 'structured');
            testCase.verifyEqual(actual_struct, testCase.expected_struct);
        end
    end
end
