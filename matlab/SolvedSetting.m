classdef SolvedSetting < handle
    %SOLVEDSETTING Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(SetAccess=private, GetAccess=public)
        Parties
        Setting
        SolvedMomentMatrix
    end
    
    methods
        function obj = SolvedSetting(setting, argA, argB, argC)
            %SOLVEDSETTING Construct an instance of this class.
            % From either (Setting, SolvedMomentMatrix) or
            % (Setting, MomentMatrix, Symmetric elements, Anti-sym elements).
            arguments
                setting (1,1) Setting
                argA
                argB
                argC
            end
            
            % Save handles
            obj.Setting = setting;
            
            if nargin == 2
                if ~isa(argA, 'SolvedMomentMatrix')
                    error("SolvedSetting should be constructed from " + ...
                        "Setting, SolvedMomentMatrix, or from " + ...
                        "Setting, MomentMatrix, symmetric basis elements " + ...
                        ", (anti-symmetric basis elements).");
                end
                obj.SolvedMomentMatrix = argA;
            elseif nargin >= 3 && nargin <= 4
                if ~isa(argA, 'MomentMatrix')
                    error("SolvedSetting should be constructed from " + ...
                        "Setting, SolvedMomentMatrix, or from " + ...
                        "Setting, MomentMatrix, symmetric basis elements " + ...
                        ", (anti-symmetric basis elements).");
                end
                if nargin == 3
                    obj.SolvedMomentMatrix = SolvedMomentMatrix(argA, argB);
                else
                    obj.SolvedMomentMatrix = ...
                        SolvedMomentMatrix(argA, argB, argC);
                end
            end
            
            
            obj.applySolutionToSetting();
        end
    end
    
    methods
        function val = get(obj, what)
            arguments
                obj (1,1) SolvedSetting
                what
            end
            if isa(what, 'Setting.Party')
                obj.checkSetting(what);
                val = obj.Parties(what.Id);
            elseif isa(what, 'Setting.Measurement')
                obj.checkSetting(what);
                val = obj.Parties(what.Index(1)).Measurements(what.Index(2));
            elseif isa(what, 'Setting.Outcome')
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
            joint_object = size(index, 1) > 1;
            
            if joint_object
                joint_what = size(index, 2);
                if joint_what == 2
                    found_jm = obj.Setting.get(index);
                    val = SolvedSetting.SolvedJointMeasurement(obj, ...
                                        found_jm);
                else
                    error("Could not retrieve joint object with "...
                          + joint_what + " indices per object.");
                end
            else
                found = obj.Setting.get(index);
                val = obj.get(found);
            end
        end
        
        function val = Value(obj, thing)
            arguments
                obj (1,1) SolvedSetting
                thing (1,1) RealObject
            end
            val = obj.SolvedMomentMatrix.Value(thing);
        end
    end
    
    methods(Access=private)
        function applySolutionToSetting(obj)
            arguments
                obj (1,1) SolvedSetting
            end
            import SolvedSetting.SolvedParty;
            
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

