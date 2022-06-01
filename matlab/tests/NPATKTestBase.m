classdef NPATKTestBase  < matlab.unittest.TestCase
    %NPATKTESTBASE Common set-up and tear-down methods for NPATK tests.
    
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
end

