classdef SolvedFullCorrelator < handle
    %SOLVEDFULLCORRELATOR 
    
    properties(SetAccess=private, GetAccess=public)
        SolvedScenario
        Values
    end
    
    methods
        function obj = SolvedFullCorrelator(scenario)
            arguments
                scenario (1,1) SolvedScenario.SolvedScenario
            end
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
            disp(coef_mono)
            values = reshape(coef_mono * obj.SolvedScenario.RealValues,...
                             fc.Shape);

        end
    end
end

