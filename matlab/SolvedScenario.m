classdef SolvedScenario< handle
    %SOLVEDSCENARIO A scenario, associated with a solved moment matrix.
    %   
      
    properties(SetAccess=private, GetAccess=public)
        Parties
        Scenario
        SolvedMomentMatrix
    end
    
    methods
        function obj = SolvedScenario(setting, argA, argB, argC)
            %SOLVEDSETTING Construct an instance of this class.
            % From either (Scenario, SolvedMomentMatrix) or
            % (Scenario, MomentMatrix, Symmetric elements, Anti-sym elements).
            arguments
                setting (1,1) Scenario
                argA
                argB = 0
                argC = 0
            end
            
            % Save handles
            obj.Scenario = setting;
            
            if nargin == 2
                if ~isa(argA, 'SolvedMomentMatrix')
                    error("SolvedScenario should be constructed from " + ...
                        "Scenario, SolvedMomentMatrix, or from " + ...
                        "Scenario, MomentMatrix, symmetric basis elements " + ...
                        ", (anti-symmetric basis elements).");
                end
                obj.SolvedMomentMatrix = argA;
            elseif nargin >= 3 && nargin <= 4
                if ~isa(argA, 'MomentMatrix')
                    error("SolvedScenario should be constructed from " + ...
                        "Scenario, SolvedMomentMatrix, or from " + ...
                        "Scenario, MomentMatrix, symmetric basis elements " + ...
                        ", (anti-symmetric basis elements).");
                end
                if nargin == 3
                    obj.SolvedMomentMatrix = SolvedMomentMatrix(argA, argB);
                else
                    obj.SolvedMomentMatrix = ...
                        SolvedMomentMatrix(argA, argB, argC);
                end
            end
            
            
            obj.applySolutionToScenario();
        end

        function val = get(obj, what)
            arguments
                obj (1,1) SolvedScenario
                what
            end
            if isa(what, 'Scenario.Party')
                obj.checkScenario(what);
                val = obj.Parties(what.Id);
            elseif isa(what, 'Scenario.Measurement')
                obj.checkScenario(what);
                val = obj.Parties(what.Index(1)).Measurements(what.Index(2));
            elseif isa(what, 'Scenario.Outcome')
                obj.checkScenario(what);
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
                obj (1,1) SolvedScenario
                index (:,:) uint64
            end
            joint_object = size(index, 1) > 1;
            
            if joint_object
                joint_what = size(index, 2);
                if joint_what == 2
                    found_jm = obj.Scenario.get(index);
                    val = SolvedScenario.SolvedJointMeasurement(obj, ...
                                        found_jm);
                else
                    error("Could not retrieve joint object with "...
                          + joint_what + " indices per object.");
                end
            else
                found = obj.Scenario.get(index);
                val = obj.get(found);
            end
        end
        
        function val = Value(obj, thing)
            arguments
                obj (1,1) SolvedScenario
                thing (1,1) RealObject
            end
            val = obj.SolvedMomentMatrix.Value(thing);
        end
        
        function val = FullCorrelator(obj)
            arguments
                obj (1,1) SolvedScenario
            end
            val = SolvedScenario.SolvedFullCorrelator(obj);
        end
    end
    
    methods(Access=private)
        function applySolutionToScenario(obj)
            arguments
                obj (1,1) SolvedScenario
            end
            import SolvedScenario.SolvedParty;
            
            obj.Parties = SolvedParty.empty;
            for party = obj.Scenario.Parties
                obj.Parties(end+1) = SolvedParty(obj.SolvedMomentMatrix,...
                    party);
            end
        end
        
        function checkScenario(obj, what)
            if what.Scenario ~= obj.Scenario
                error("Scenario of input must match SolvedScenario's.");
            end
        end
    end
end

