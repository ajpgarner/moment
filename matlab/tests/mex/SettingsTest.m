classdef SettingsTest < MTKTestBase
    %SETTTINGSTESTS Unit tests for settings mex function
    methods (Test, TestTags={'mex'})
        function ReportSettings(testCase)
            the_settings = mtk('settings', 'structured');
            testCase.assertTrue(isa(the_settings, 'struct'));
            testCase.verifyTrue(isfield(the_settings, 'locality_format'));
        end
        
        function SetByStruct(testCase)
            requested_settings = struct('locality_format', 'Natural');
            mtk('settings', requested_settings);
            
            the_settings = mtk('settings', 'structured');
            testCase.assertTrue(isa(the_settings, 'struct'));
            testCase.assertTrue(isfield(the_settings, 'locality_format'));
            testCase.verifyEqual(the_settings.locality_format, ...
                "Natural");
        end
        
        function SetLF(testCase)
            mtk('settings', 'locality_format', 'Traditional');
            
            the_settings = mtk('settings', 'structured');
            testCase.assertTrue(isa(the_settings, 'struct'));
            testCase.assertTrue(isfield(the_settings, 'locality_format'));
            testCase.verifyEqual(the_settings.locality_format, ...
                "Traditional");
            
            mtk('settings', 'locality_format', 'Natural');
            
            the_settings2 = mtk('settings', 'structured');
            testCase.assertTrue(isa(the_settings2, 'struct'));
            testCase.assertTrue(isfield(the_settings2, 'locality_format'));
            testCase.verifyEqual(the_settings2.locality_format, ...
                "Natural");
        end
        
        
    end
    
    methods (Test, TestTags={'Mex', 'Error'})
        function Error_BadInput(testCase)
            function no_in()
                [~] = mtk('settings', 'not a setting');
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadLFMode(testCase)
            function no_in()
                [~] = mtk('settings', 'locality_format', 'mysterious');
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_DoubleSpecification(testCase)
            function no_in()
                sIn = struct('locality_format', 'natural');
                [~] = mtk('settings', sIn, 'locality_format', 'traditional');
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
    end
end