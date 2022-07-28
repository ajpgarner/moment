classdef Party < handle
    %PARTY A collection of localized operators.
    %   
    
    properties(GetAccess = public, SetAccess = protected)
        Id
        Name
        RawOperators
        Measurements
        Setting
    end
    
    methods(Access={?Setting})
        function obj = Party(setting, id, name, raw)
            arguments
                setting (1,1) Setting
                id (1,1) uint64 {mustBeInteger, mustBePositive}
                name (1,1) string = ""
                raw (1,1) uint64 {mustBeInteger, mustBeNonnegative} = 0
            end
            %PARTY Construct a party 
            % (Private c'tor. To construct, use Setting.AddParty.)
            import Setting.Measurement
            
            % Link to a setting object
            obj.Setting = setting;
            
            % Supply an index
            obj.Id = id;
            
            % Set name, or automatically generate one
            if nargin >= 3            
                obj.Name = string(name);
            else
                obj.Name = string(alphabetic_index(obj.Id, true));
            end
            
            % Add operators that are not grouped in a measurement, if any
            if nargin >= 4
                obj.RawOperators = uint64(raw);
            else
                obj.RawOperators = uint64(0);
            end
            
            % Prepare (empty) measurement array
            obj.Measurements = Measurement.empty;
        end
    end
    
    methods
        function mmt = AddMeasurement(obj, num_outcomes, name)
            arguments
                obj Setting.Party                
                num_outcomes (1,1) uint64 {mustBeInteger, mustBePositive}
                name (1,1) string = ""
            end
            import Setting.Measurement
            
            % Automatically name, if non supplied
            next_id = length(obj.Measurements)+1;
            if nargin < 3                
                name = alphabetic_index(next_id, false);
            end
                
            obj.Measurements(end+1) = Measurement(obj.Setting, ...
                                                  obj.Id, next_id, ...
                                                  string(name), ...
                                                  num_outcomes);
            % Return newly created measurement
            mmt = obj.Measurements(end);
        end
    end
end

