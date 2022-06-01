classdef VersionTest < NPATKTestBase
    %VERSIONTEST Unit tests for version function
    
    properties(Constant)
        expected_string = '0.1.0';
        expected_struct = struct('major', int64(0), ...
                                 'minor', int64(1), ...
                                 'build', int64(0));
    end

    methods (Test)
        function Version_String(testCase)
            actual_string = npatk('version');
            testCase.verifyEqual(actual_string, testCase.expected_string);
        end
        
        function Version_Struct(testCase)
            actual_struct = npatk('version', 'structured');
            testCase.verifyEqual(actual_struct, testCase.expected_struct);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_MissingParam(testCase)
            testCase.verifyError(@() npatk('version', 'cake'), ...
                'npatk:bad_param');
        end
        
        function Error_Mutex(testCase)
            testCase.verifyError(@() npatk('version', 'foo', 'bar'), ...
                'npatk:mutex_param');
        end
        
        function Error_Mutex2(testCase)
            testCase.verifyError(@() npatk('version', 'foo', ...
                                           'cake', 1), ...
                                           'npatk:mutex_param');
        end
    end
end
