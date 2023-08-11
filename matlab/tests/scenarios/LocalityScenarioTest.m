classdef LocalityScenarioTest < MTKTestBase
%ALGEBRAICTEST Tests for AlgebraicScenario object.
        
%% Construction / binding
    methods(Test, TestTags={'MTKScenario'})
        function FCTensor(testCase)
            setting = LocalityScenario(2, 2, 2);
            
            [A0, A1, B0, B1] = setting.getMeasurements;                       
            poly_fc = setting.FCTensor([1 2 3; 4 5 6; 7 8 9]);            
            poly_ref = 1 + 2 * Expt(B0) + 3 * Expt(B1) ...
                     + 4 * Expt(A0) + 5 * Correlator(A0, B0) + 6 * Correlator(A0, B1) ...
                     + 7 * Expt(A1) + 8 * Correlator(A1, B0) + 9 * Correlator(A1, B1);
            
             testCase.verifyEqual(poly_fc, poly_ref);            
        end        
    end
end
