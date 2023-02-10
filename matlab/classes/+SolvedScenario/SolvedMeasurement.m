classdef SolvedMeasurement < handle
    %SOLVEDMEASUREMENT Summary of this class goes here
    %   Detailed explanation goes here
    properties(SetAccess=private, GetAccess=public)
        SolvedScenario
        Measurement
        Outcomes
        Distribution
    end
    
    methods
        function obj = SolvedMeasurement(solved_scenario, measurement)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                solved_scenario (1,1) SolvedScenario.SolvedScenario
                measurement (1,1) Locality.Measurement
            end
            import SolvedScenario.SolvedOutcome;
            
            obj.SolvedScenario = solved_scenario;
            obj.Measurement = measurement;            
            obj.Outcomes = SolvedOutcome.empty;
            obj.Distribution = zeros(length(obj.Measurement.Outcomes), 1);
            for i = 1:length(obj.Measurement.Outcomes)
                outcome = obj.Measurement.Outcomes(i);
                obj.Outcomes(end+1) = SolvedOutcome(solved_scenario, outcome);
                obj.Distribution(i) = obj.Outcomes(end).Probability;
            end
        end
    end
end

