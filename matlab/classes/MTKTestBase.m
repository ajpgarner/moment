classdef MTKTestBase  < matlab.unittest.TestCase
    %MTKTESTBASE Common set-up and tear-down methods for MTK tests.
    
    methods(TestMethodSetup)
        function addMTKpath(testCase)
            import matlab.unittest.fixtures.PathFixture
            testCase.applyFixture(PathFixture([".."]));
        end
    end
    
    methods(TestMethodTeardown)
        function clearMTK(testCase)
            clear mtk
        end
    end
end

