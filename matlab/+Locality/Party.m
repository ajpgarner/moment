classdef Party < handle
    %PARTY A collection of localized operators.
    %   
    
    properties(GetAccess = public, SetAccess = private)
        Id
        Name
        TotalOperators = uint64(0)
        Measurements
        Scenario
    end
    
    methods(Access={?LocalityScenario})
        function obj = Party(setting, id, name)
            arguments
                setting (1,1) LocalityScenario
                id (1,1) uint64 {mustBeInteger, mustBePositive}
                name (1,1) string = ""
            end
            %PARTY Construct a party 
            % (Private c'tor. To construct, use Scenario.AddParty.)
            import Locality.Measurement
            import Util.alphabetic_index
            
            % Link to a setting object
            obj.Scenario = setting;
            
            % Supply an index
            obj.Id = id;
            
            % Set name, or automatically generate one
            if nargin >= 3            
                obj.Name = string(name);
            else
                obj.Name = string(alphabetic_index(obj.Id, true));
            end
            
            % No operators until we have measurements
            obj.TotalOperators = uint64(0);
            
            % Prepare (empty) measurement array
            obj.Measurements = Measurement.empty;
        end
    end
    
    methods
        function mmt = AddMeasurement(obj, num_outcomes, name)
            arguments
                obj Locality.Party                
                num_outcomes (1,1) uint64 {mustBeInteger, mustBePositive}
                name (1,1) string = ""
            end
            import Locality.Measurement
            import Util.alphabetic_index
            
            % Check not locked
            obj.Scenario.errorIfLocked();
            
            % Automatically name, if non supplied
            next_id = length(obj.Measurements)+1;
            if nargin < 3                
                name = alphabetic_index(next_id, false);
            end
                
            obj.Measurements(end+1) = Measurement(obj.Scenario, ...
                                                  obj.Id, next_id, ...
                                                  string(name), ...
                                                  num_outcomes);

            % Return newly created measurement
            mmt = obj.Measurements(end);
            
            % Add to total operator count (1 fewer than number of outcomes)
            obj.TotalOperators = obj.TotalOperators + num_outcomes - 1;
                                              
            % Build joint measurements, if any other measurements
            obj.Scenario.make_joint_mmts(obj.Id, mmt);
        end
    end
end
