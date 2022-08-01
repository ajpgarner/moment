classdef JointMeasurement < handle
    %JOINTMEASUREMENT Collection of two or more measurements
    properties(SetAccess=private, GetAccess=public)
        Scenario
        Marginals
        Indices
        Shape
    end
    
    methods
        function obj = JointMeasurement(setting, measurements)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                setting (1,1) Scenario
                measurements (1, :) Scenario.Measurement
            end
            
            % Get setting
            obj.Scenario = setting;
                                    
            % Get marginal measurements
            obj.Marginals = obj.checkAndSortMmts(measurements);
            
            % Get (sorted) measurement indices:
            obj.Indices = uint64.empty(0,2);
            for mmt = obj.Marginals
                obj.Indices(end+1, :) = mmt.Index;
            end
            
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
                error("Correlator only defined for two-measurements.");
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
        end
    end
end

