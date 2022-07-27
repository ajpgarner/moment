classdef SolvedSetting < handle
    %SOLVEDSETTING Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(SetAccess=private, GetAccess=public)
        SolvedMomentMatrix
        Parties
        Setting
    end
    
    methods
        function obj = SolvedSetting(solvedMM, setting)
            %SOLVEDSETTING Construct an instance of this class
            arguments
                solvedMM (1,1) SolvedMomentMatrix
                setting (1,1) Setting
            end
                            
            % Save handles
            obj.SolvedMomentMatrix = solvedMM;
            obj.Setting = setting;
            
            obj.applySolutionToSetting();            
        end
    end
    
    methods
        function val = get(obj, what) 
            arguments
                obj (1,1) SolvedSetting
                what
            end
            if isa(what, 'Party')
                obj.checkSetting(what);
                val = obj.Parties(what.Id);
            elseif isa(what, 'Measurement')
                obj.checkSetting(what);
                val = obj.Parties(what.Index(1)).Measurements(what.Index(2));
            elseif isa(what, 'Outcome')
                obj.checkSetting(what);
                val = obj.Parties(what.Index(1)).Measurements(what.Index(2)).Outcomes(what.Index(3));
            elseif isnumeric(what)
                val = obj.getByIndex(what);
            else
                error("Cannot associated a solved object with an "...
                      + "object of type " + class(what));
            end
        end
        
        function val = getByIndex(obj, index)
            arguments
                obj (1,1) SolvedSetting
                index (:,:) uint64
            end
            found = obj.Setting.get(index);
            val = obj.get(found);
        end    
        
    end
    
    methods(Access=private)
        function applySolutionToSetting(obj)
            arguments
                obj (1,1) SolvedSetting
            end
            obj.Parties = SolvedParty.empty;
            for party = obj.Setting.Parties
                obj.Parties(end+1) = SolvedParty(obj.SolvedMomentMatrix,...
                                                 party);
            end
        end
        
        function checkSetting(obj, what)
            if what.Setting ~= obj.Setting
                error("Setting of input must match that of SolvedSetting.");
            end
        end
    end
end

