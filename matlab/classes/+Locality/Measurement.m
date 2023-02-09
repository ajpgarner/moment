classdef Measurement < handle & RealObject
    %MEASUREMENT A collection of outcomes with assigned values
    
    properties(SetAccess=private, GetAccess=public)
        Id
        Index
        Name
        Outcomes
        joint_mmts
    end
    
    properties(Access={?Scenario})
        
    end
    
    properties(Constant, Access = protected)
        err_overlapping_parties = ...
            "_*_ can only be used to form linear combinations of "...
            + "probabilities (i.e. all operands must be from different "...
            + "parties).";
    end
    
    methods
        function obj = Measurement(scenario, party_index, mmt_index, ...
                                   name, num_outcomes, values)
            arguments
                scenario (1,1) LocalityScenario
                party_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                mmt_index (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                name (1,1) string
                num_outcomes (1,1) uint64 {mustBeInteger, mustBeNonnegative}
                values (1,:) double = double.empty(1,0)
            end
            import Locality.Outcome
            
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
                obj (1,1) Locality.Measurement
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
        
        function val = ExplicitValues(obj, distribution)
            arguments
                obj Locality.Measurement
                distribution (1,:) double
            end
            if length(distribution) ~= length(obj.Outcomes)
                error("Distribution must match number of outcomes.");
            end
            val = mtk('make_explicit', obj.Scenario.System.RefId, ...
                      obj.Index, distribution);
        end
        
        function joint_item = mtimes(objA, objB)
            arguments
                objA (1,1)
                objB (1,1)
            end
                        
            % Should only occur when A is a built-in object (e.g. scalar)
            if ~isa(objA, 'Locality.Measurement')
                joint_item = mtimes@RealObject(objA, objB);
                return
            end
            
            % Can multiply measurements to form joint measurements
            if isa(objB, 'Locality.Measurement')                
                if objA.Scenario ~= objB.Scenario
                    error(objA.err_mismatched_scenario);
                end
                if ~isempty(intersect(objA.Index(:,1), ...
                                      objB.Index(:,1)))
                    error(objA.err_overlapping_parties);                    
                end
                indices = sortrows(vertcat(objA.Index, objB.Index));
                joint_item = objA.Scenario.get(indices);
            elseif isa(objB, 'Locality.JointMeasurement')
                if objA.Scenario ~= objB.Scenario
                    error(objA.err_mismatched_scenario);
                end
                if ~isempty(intersect(objA.Index(:,1), ...
                                      objB.Indices(:,1)))
                    error(objA.err_overlapping_parties);                    
                end
                indices = sortrows(vertcat(objA.Index, objB.Indices));
                joint_item = objA.Scenario.get(indices);
            else
                % Fall back to superclass:~
                joint_item = mtimes@RealObject(objA, objB);
            end
        end
    end
    
    %% Overriden methods
    methods(Access={?Locality.Measurement,?LocalityScenario})
        function addJointMmt(obj, otherMmt)
            % With existing joint measurements, if a new party
            otherParty = otherMmt.Index(1);
            initial_len = length(obj.joint_mmts);
            for i = 1:initial_len 
                existing_mmt = obj.joint_mmts(i).mmt;
                if ~existing_mmt.ContainsParty(otherParty)
                    njMmt = Locality.JointMeasurement(obj.Scenario, ...
                                [existing_mmt.Marginals, otherMmt]);
                    obj.joint_mmts(end+1) = ...
                        struct('indices', njMmt.Indices, ...
                               'mmt', njMmt);
                end
            end
            
            % With this...
            jointMmt = Locality.JointMeasurement(obj.Scenario, ...
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
