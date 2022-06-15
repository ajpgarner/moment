classdef Party < handle
    %PARTY Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(GetAccess = public, SetAccess = protected)
        Id
        Name
        RawOperators
        Measurements
    end
    
    methods
        function obj = Party(id, name, raw)
            %PARTY Construct a party
            
            % Can just supply an index
            if (id < 1)
                error("Party ID must be 1 or greater.");
            end
            obj.Id = uint64(id);
            
            % Set name, or automatically generat one
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
            num_outcomes = uint64(num_outcomes);
            if nargin < 3
                next_id = length(this.Measurements)+1;
                name = alphabetic_index(next_id, false);
            end
                
            this.Measurements(end+1) = struct('name', string(name), ...
                                             'num_outcomes', num_outcomes);
        end
    end
end

