classdef SolvedParty < handle
    %SOLVEDPARTY Summary of this class goes here
    %   Detailed explanation goes here
    properties(SetAccess=private, GetAccess=public)
        SolvedMomentMatrix
        Party
        Measurements
    end
    
    methods
        function obj = SolvedParty(solvedMM, party)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                solvedMM (1,1) SolvedMomentMatrix
                party (1,1) Setting.Party
            end
            import SolvedSetting.SolvedMeasurement;
            
            obj.SolvedMomentMatrix = solvedMM;
            obj.Party = party;
            
            obj.Measurements = SolvedMeasurement.empty;
            for mmt = obj.Party.Measurements
                obj.Measurements(end+1) = SolvedMeasurement(solvedMM, ...
                                                            mmt);
            end
        end
    end
end
