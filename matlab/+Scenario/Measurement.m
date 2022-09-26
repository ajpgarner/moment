classdef Measurement < handle & RealObject
    %MEASUREMENT A collection of outcomes
    
    properties(SetAccess=private, GetAccess=public)
        Id
        Index
        Name
        Outcomes
    end
    
    properties(Access={?Scenario})
        joint_mmts
    end
    
    methods
        function obj = Measurement(scenario, party_index, mmt_index, ...
                                   name, num_outcomes, values)
            %MEASUREMENT Construct an instance of this class
            arguments
                scenario (1,1) Scenario
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                name (1,1) string
                num_outcomes (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                values (1,:) double = double.empty(1,0)
            end
            import Scenario.Outcome
            
            % Superclass c'tor
            obj = obj@RealObject(scenario);
                        
            obj.Id = mmt_index;
            obj.Index = uint64([party_index, mmt_index]);
            obj.Name = name;
            
            obj.joint_mmts = struct('indices', {}, 'mmt', {});
            
            % Check values, set default values
            if nargin <= 5
                if num_outcomes == 2
                    values = [+1, -1];
                else
                    values = 1:num_outcomes;
                end
            else
                if length(values) ~= num_outcomes
                    error("Number of outcomes must match number of supplied values")
                end
            end
            
            % Construct outcomes
            obj.Outcomes = Outcome.empty;
            for x = 1:num_outcomes
                obj.Outcomes(end+1) = Outcome(obj.Scenario, ...
                                              obj.Index(1), ...
                                              obj.Index(2), uint64(x), ...
                                              values(x));
            end
        end
        
           
        function item = JointMeasurement(obj, indices)
            arguments
                obj (1,1) Scenario.Measurement
                indices (:,2) uint64
            end
            table_index = find(arrayfun(@(s) ...
                              isequal(indices, s.indices), ...
                              obj.joint_mmts));
            if length(table_index) ~= 1
                error("Could not find joint measurement at supplied indices.");
            end
            item = obj.joint_mmts(table_index).mmt;
        end
    end
    
    %% Overriden methods
    methods(Access={?Scenario.Measurement,?Scenario})
        function addJointMmt(obj, otherMmt)
            jointMmt = Scenario.JointMeasurement(obj.Scenario, ...
                                                [obj, otherMmt]);
            obj.joint_mmts(end+1) = struct('indices', jointMmt.Indices, ...
                                           'mmt', jointMmt);
        end
    end
    
    %% Overriden methods
    methods(Access=protected)
        function calculateCoefficients(obj)               
            % Infer number of real elements:
            re_basis_size = size(obj.Outcomes(1).Coefficients, 2);
            obj.real_coefs = sparse(1, re_basis_size);
            for outcome = obj.Outcomes
                obj.real_coefs = obj.real_coefs ...
                                    + (outcome.Value * outcome.Coefficients);
            end
        end
    end
end
