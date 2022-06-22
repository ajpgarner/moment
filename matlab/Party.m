classdef Party < handle
    %PARTY A collection of localized operators.
    %   
    
    properties(GetAccess = public, SetAccess = protected)
        Id
        Name
        RawOperators
        Measurements
    end
    
    methods
        function obj = Party(id, name, raw)
            arguments
                id (1,1) uint64 {mustBeInteger, mustBePositive}
                name (1,1) string = ""
                raw (1,1) uint64 {mustBeInteger, mustBeNonnegative} = 0
            end
            %PARTY Construct a party
            
            % Supply an index
            obj.Id = id;
            
            % Set name, or automatically generate one
            if nargin >= 2            
                obj.Name = string(name);
            else
                obj.Name = string(alphabetic_index(obj.Id, true));
            end
            
            % Add operators that are not grouped in a measurement, if any
            if nargin >= 3
                obj.RawOperators = uint64(raw);
            else
                obj.RawOperators = uint64(0);
            end
            
            % Prepare (empty) measurement struct
            obj.Measurements = struct('name', {}, 'num_outcomes', {});
        end
        
        function AddMeasurement(this, num_outcomes, name)
            arguments
                this Party                
                num_outcomes (1,1) uint64 {mustBeInteger, mustBePositive}
                name (1,1) string = ""
            end
            
            % Automatically name, if non supplied
            if nargin < 3
                next_id = length(this.Measurements)+1;
                name = alphabetic_index(next_id, false);
            end
                
            this.Measurements(end+1) = struct('name', string(name), ...
                                             'num_outcomes', num_outcomes);
        end
    end
end

