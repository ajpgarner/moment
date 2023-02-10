classdef SolvedParty < handle
    %SOLVEDPARTY Summary of this class goes here
    %   Detailed explanation goes here
    properties(SetAccess=private, GetAccess=public)
        SolvedScenario
        Party
        Measurements
    end
    
    methods
        function obj = SolvedParty(solved_scenario, party)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                solved_scenario (1,1) SolvedScenario.SolvedScenario
                party (1,1) Locality.Party
            end
            import SolvedScenario.SolvedMeasurement;
            
            obj.SolvedScenario = solved_scenario;
            obj.Party = party;            
            obj.Measurements = SolvedMeasurement.empty;
            for mmt = obj.Party.Measurements
                obj.Measurements(end+1) = ...
                    SolvedMeasurement(solved_scenario, mmt);
            end
        end
    end
end
