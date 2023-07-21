classdef Party < handle
    %PARTY A collection of localized operators.
    %   
    
    properties(GetAccess = public, SetAccess = private)
        Scenario
        Id
        Name
        TotalOperators = uint64(0)
        Measurements
    end
    
    methods(Access={?LocalityScenario})
        function obj = Party(setting, id, name)
        %PARTY Construct a party 
        % (Private c'tor. To construct, use Scenario.AddParty.)
            
			% Validate:
			assert(nargin>=2 && isa(setting, 'LocalityScenario'));
			id = uint64(id);
			if nargin<3
				name = "";
			end
            
            import MTKUtil.alphabetic_index
            
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
            obj.Measurements = Locality.Measurement.empty(0,1);
        end
    end
    
    methods
        function mmt = AddMeasurement(obj, num_outcomes, name)
            
			% Validate
			if (nargin < 2) || (num_outcomes <= 0)
				error("Must specify positive number of outcomes for party.");
			else
				num_outcomes = uint64(num_outcomes);
			end
			
			if nargin < 3
				name = "";
			end
			
            import Locality.Measurement
            import MTKUtil.alphabetic_index
            
            % Check not locked
            obj.Scenario.errorIfLocked();
            
            % Automatically name, if non supplied
            next_id = length(obj.Measurements)+1;
            if nargin < 3
                format = mtk_locality_format();
                if strcmp(format, "Natural")
                    name = string(alphabetic_index(next_id, false));    
                else
                    name = num2str(next_id- 1);
                end
            end
                
            obj.Measurements(end+1) = Measurement(obj.Scenario, ...
                                                  obj.Id, next_id, ...
                                                  string(name), ...
                                                  num_outcomes);

            % Return newly created measurement
            mmt = obj.Measurements(end);
            
            % Add to total operator count (1 fewer than number of outcomes)
            obj.TotalOperators = obj.TotalOperators + num_outcomes - 1;
        end
    end
end

