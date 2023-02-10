classdef SolvedLocalityScenario < SolvedScenario.SolvedScenario
    %SOLVEDLOCALITYSCENARIO 
      
    properties(SetAccess=private, GetAccess=public)
        Parties = SolvedScenario.SolvedParty.empty(1,0);
    end
     
    methods
        function obj = SolvedLocalityScenario(setting, real_vals, im_vals)
            %SOLVEDLOCALITYSCENARIO 
            
            obj = obj@SolvedScenario.SolvedScenario(setting, ...
                                                    real_vals, im_vals);            
            obj.applySolutionToScenario();
        end        
           
        function val = FullCorrelator(obj)
            arguments
                obj (1,1) SolvedScenario.SolvedLocalityScenario
            end
            val = SolvedScenario.SolvedFullCorrelator(obj);
        end
        
        function val = Get(obj, what)
            arguments
                obj (1,1) SolvedScenario.SolvedLocalityScenario
                what
            end
            
            if isa(what, 'Locality.Party')
                obj.checkScenario(what);
                val = obj.Parties(what.Id);
            elseif isa(what, 'Locality.Measurement')
                obj.checkScenario(what);
                val = obj.Parties(what.Index(1)).Measurements(what.Index(2));
            elseif isa(what, 'Locality.JointMeasurement')
                obj.checkScenario(what)
                val = SolvedScenario.SolvedJointMeasurement(obj, what);
            elseif isa(what, 'Locality.Outcome')
                obj.checkScenario(what);
                val = obj.Parties(what.Index(1)).Measurements(what.Index(2)).Outcomes(what.Index(3));
            elseif isnumeric(what)
                val = obj.getByIndex(what);
            else
                error("Cannot associate a solved object with an "...
                    + "object of type %s.", class(what));
            end
        end
                
        function val = GetByIndex(obj, index)
            arguments
                obj (1,1) SolvedScenario.SolvedLocalityScenario
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
    end
    
    methods(Access=private)
        function applySolutionToScenario(obj)
            arguments
                obj (1,1) SolvedScenario.SolvedLocalityScenario
            end
            import SolvedScenario.SolvedParty;
                        
            obj.Parties = SolvedParty.empty;
            for party = obj.Scenario.Parties
                obj.Parties(end+1) = ...
                    SolvedParty(obj, party);
            end
        end
    end
end

