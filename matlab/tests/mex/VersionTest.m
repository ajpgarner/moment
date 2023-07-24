classdef VersionTest < MTKTestBase
    %VERSIONTEST Unit tests for version function
    
    properties(Constant)
        expected_string = '0.1.1';
        expected_struct = struct('major', int64(0), ...
                                 'minor', int64(1), ...
                                 'patch', int64(1));
    end

    methods (Test, TestTags={'mex'})
        function Version_String(testCase)
            actual_string = mtk('version');
            testCase.verifyEqual(actual_string, testCase.expected_string);
        end
        
        function Version_Struct(testCase)
            actual_struct = mtk('version', 'structured');
            testCase.verifyEqual(actual_struct, testCase.expected_struct);
        end
    end
    
    methods (Test, TestTags={'Mex', 'Error'})
        function Error_MissingParam(testCase)
            testCase.verifyError(@() mtk('version', 'cake'), ...
                'mtk:bad_param');
        end
        
        function Error_Mutex(testCase)
            testCase.verifyError(@() mtk('version', 'foo', 'bar'), ...
                'mtk:mutex_param');
        end
        
        function Error_Mutex2(testCase)
            testCase.verifyError(@() mtk('version', 'foo', ...
                                           'cake', 1), ...
                                           'mtk:mutex_param');
        end
        
        function Error_Mutex3(testCase)
            testCase.verifyError(@() mtk('version', 'alice', ...
                                           'bob'), ...
                                           'mtk:mutex_param');
        end
    end
end
