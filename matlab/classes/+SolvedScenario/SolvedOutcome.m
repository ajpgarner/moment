classdef SolvedOutcome < handle
    %SOLVEDOUTCOME Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        SolvedSetting
        Outcome
        Probability
    end
    
     methods
        function obj = SolvedOutcome(solved_setting, outcome)
            %SOLVEDOUTCOME Construct an instance of this class
            arguments
                solved_setting (1,1) SolvedScenario.SolvedScenario
                outcome (1,1) Locality.Outcome
            end
            
            obj.SolvedSetting = solved_setting;
            obj.Outcome = outcome;
            
            obj.Probability = obj.Outcome.Apply(solved_setting.RealValues);
        end
    end
end

