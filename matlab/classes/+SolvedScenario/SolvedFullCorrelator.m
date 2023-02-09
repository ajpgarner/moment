classdef SolvedFullCorrelator < handle
    %SOLVEDFULLCORRELATOR Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(SetAccess=private, GetAccess=public)
        SolvedMomentMatrix
        SolvedScenario
        Values
    end
    
    methods
        function obj = SolvedFullCorrelator(scenario)
            arguments
                scenario (1,1) SolvedScenario
            end
            obj.SolvedMomentMatrix = scenario.SolvedMomentMatrix;
            obj.SolvedScenario = scenario;
            obj.Values = obj.calculateValues();
        end
    end
    
    methods(Access=private)
        function values = calculateValues(obj)
            arguments
                obj (1,1) SolvedScenario.SolvedFullCorrelator
            end
            
            scenario = obj.SolvedScenario.Scenario;
            fc = Locality.FullCorrelator(scenario);
            
            % Build monolith of co-efficients
            coef_mono = fc.Coefficients;
            values = reshape(coef_mono * obj.SolvedMomentMatrix.a, fc.Shape);

        end
    end
end

