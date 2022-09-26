classdef JointMeasurement < handle & RealObject
    %JOINTMEASUREMENT Collection of two or more measurements
    properties(SetAccess=private, GetAccess=public)
        Marginals
        Indices
        Shape
        Values
    end
    
    methods
        function obj = JointMeasurement(scenario, measurements)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                scenario (1,1) Scenario
                measurements (1, :) Scenario.Measurement
            end
            
            % Set scenario in superclass c'tor
            obj = obj@RealObject(scenario);
                                    
            % Get marginal measurements
            obj.Marginals = obj.checkAndSortMmts(measurements);
            
            % Get (sorted) measurement indices:
            obj.Indices = uint64.empty(0,2);
            for mmt = obj.Marginals
                obj.Indices(end+1, :) = mmt.Index;
            end
            
            % Build joint array
            obj.buildJointOutcomeArray();            
        end
        
        function val = Outcome(obj, index)
            arguments
                obj (1,1) Scenario.JointMeasurement
                index (1, :) uint64
            end
            if length(index) ~= length(obj.Shape)
                error("Index should have " + length(obj.Shape) + " elements");
            end
            if any(index > obj.Shape)
                error("Index out of bounds.")
            end
            
            indices = horzcat(obj.Indices, ...
                              reshape(index, [length(obj.Shape), 1]));
              
            val = obj.Scenario.get(indices);
        end
        
        function val = Correlator(obj)
            if (length(obj.Shape) ~= 2)
                error("Correlator only defined for two measurements.");
            end
            val = Correlator(obj.Marginals(1), obj.Marginals(2));
        end
    end
    
    methods(Access=private)
        function sorted = checkAndSortMmts(obj, unsorted)
            arguments
                obj (1,1) Scenario.JointMeasurement
                unsorted (1,:) Scenario.Measurement
            end
            
            % Check number of measurements
            if (length(unsorted) <= 1)
                error("At least two measurements should be supplied.");
            end
            
            % Get party indices
            indices = zeros(size(unsorted));
            for i = 1:length(unsorted)
                indices(i) = unsorted(i).Index(1);
            end
            
            % Check for duplicates
            if length(indices) ~= length(unique(indices))
                error("Each measurement must be from a different party.");
            end
            
            % Sort measurements by party
            [~, sortIndex] = sort(indices);
            sorted = Scenario.Measurement.empty;
            for i = 1:length(unsorted)
                sorted(end+1) = unsorted(sortIndex(i));
            end
            
        end
        
        function buildJointOutcomeArray(obj)
            % Get number of outcomes, per measurement
            dims = uint64(zeros(1, length(obj.Marginals)));
            for mmt_index = 1:length(obj.Marginals)
                mmt = obj.Marginals(mmt_index);
                dims(mmt_index) = length(mmt.Outcomes);
            end
            obj.Shape = dims;
            
            % Calculate product values
            obj.Values = zeros(obj.Shape);
            for vIndex = 1:prod(obj.Shape)
                indices = Util.index_to_sub(obj.Shape, uint64(vIndex));
                val = 1;
                for i=1:length(indices)
                    val = val * obj.Marginals(i).Outcomes(indices(i)).Value;
                end
                obj.Values(vIndex) = val;
            end
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function calculateCoefficients(obj)
            coefs = zeros(size(obj.Scenario.Normalization.Coefficients));
            
            for index = 1:prod(obj.Shape)
                oc_index = reshape(Util.index_to_sub(obj.Shape, uint64(index)), ...
                                   [length(obj.Shape), 1]);
                full_index = horzcat(obj.Indices, oc_index);
                joint_outcome = obj.Scenario.get(full_index);
                local_coefs = joint_outcome.Coefficients * obj.Values(index);
                coefs = coefs + local_coefs;
            end
            obj.real_coefs = coefs;
        end
    end
end

