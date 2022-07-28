classdef SolvedOutcome < handle
    %SOLVEDOUTCOME Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        SolvedMomentMatrix
        Outcome
        Probability
    end
    
     methods
        function obj = SolvedOutcome(solvedMM, outcome)
            %SOLVEDOUTCOME Construct an instance of this class
            arguments
                solvedMM (1,1) SolvedMomentMatrix
                outcome (1,1) Setting.Outcome
            end
            
            obj.SolvedMomentMatrix = solvedMM;
            obj.Outcome = outcome;
            
            obj.Probability = obj.Outcome.apply(solvedMM.a);
        end
    end
end

