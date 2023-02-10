classdef SolvedJointMeasurement < handle
    %SOLVEDJOINTMEASUREMENT Summary of this class goes here
    %   Detailed explanation goes here
    properties(SetAccess=private, GetAccess=public)
        SolvedScenario
        Marginals
        Indices
        Distribution
        ExpectationValue
    end
    
    methods
        function obj = SolvedJointMeasurement(solvedScenario, jointMmt)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                solvedScenario (1,1) SolvedScenario.SolvedLocalityScenario
                jointMmt (1,1) Locality.JointMeasurement
            end
           
            % Get handle to scenario
            obj.SolvedScenario = solvedScenario;
                        
            % Get marginal measurements
            obj.Marginals = obj.getMmts(solvedScenario, jointMmt);
            
            % Get (sorted) measurement indices:
            obj.Indices = jointMmt.Indices;
            
            % Query for correlation table
            obj.Distribution = obj.queryOutcomes(jointMmt);
            
            % Query for EV
            obj.ExpectationValue = solvedScenario.Value(jointMmt);
        end
    end
    
    methods(Access=private)
        function sorted = getMmts(obj, solvedScenario, mmt)
            arguments
                obj (1,1) SolvedScenario.SolvedJointMeasurement
                solvedScenario (1,1) SolvedScenario.SolvedLocalityScenario
                mmt (1,1) Locality.JointMeasurement
            end
            
            % Check number of measurement
            sorted = SolvedScenario.SolvedMeasurement.empty;
            for i = 1:length(mmt.Marginals)
                sorted(end+1) = solvedScenario.Get(mmt.Marginals(i));
            end
        end
        
        function val = queryOutcomes(obj, jointMmt)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                obj (1,1) SolvedScenario.SolvedJointMeasurement
                jointMmt (1,1) Locality.JointMeasurement
            end
            
            % Get number of outcomes, per measurement
            dims = uint64(zeros(1, length(obj.Marginals)));
            for mmt_index = 1:length(obj.Marginals)
                mmt = obj.Marginals(mmt_index);
                dims(mmt_index) = length(mmt.Measurement.Outcomes);
            end
            
            % Prepare outputs
            max_index = prod(dims);
            val = zeros(1, max_index);
            
            
            % Iterate through outcomes
            for index = 1:max_index
                outcome_numbers = reshape(Util.index_to_sub(dims, index),...
                                          [length(dims), 1]);
                query_index = [obj.Indices, outcome_numbers];
                joint_outcome = obj.SolvedScenario.Scenario.get(query_index);
                val(index) = obj.SolvedScenario.Value(joint_outcome);

            end
            val = reshape(val, dims);
        end
    end
end

