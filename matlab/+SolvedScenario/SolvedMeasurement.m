classdef SolvedMeasurement < handle
    %SOLVEDMEASUREMENT Summary of this class goes here
    %   Detailed explanation goes here
    properties(SetAccess=private, GetAccess=public)
        SolvedMomentMatrix
        Measurement
        Outcomes
        Distribution
    end
    
    methods
        function obj = SolvedMeasurement(solvedMM, measurement)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                solvedMM (1,1) SolvedMomentMatrix
                measurement (1,1) Scenario.Measurement
            end
            import SolvedScenario.SolvedOutcome;
            
            obj.SolvedMomentMatrix = solvedMM;
            obj.Measurement = measurement;
            
            obj.Outcomes = SolvedOutcome.empty;
            obj.Distribution = zeros(length(obj.Measurement.Outcomes), 1);
            for i = 1:length(obj.Measurement.Outcomes)
                outcome = obj.Measurement.Outcomes(i);
                obj.Outcomes(end+1) = SolvedOutcome(solvedMM, outcome);
                obj.Distribution(i) = obj.Outcomes(end).Probability;
            end
        end
    end
end

