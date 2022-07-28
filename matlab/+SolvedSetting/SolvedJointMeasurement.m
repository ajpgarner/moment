classdef SolvedJointMeasurement < handle
    %SOLVEDJOINTMEASUREMENT Summary of this class goes here
    %   Detailed explanation goes here
    properties(SetAccess=private, GetAccess=public)
        SolvedMomentMatrix
        Marginals
        Indices
        Correlations
        Sequences
    end
    
    methods
        function obj = SolvedJointMeasurement(solvedSetting, measurements)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                solvedSetting (1,1) SolvedSetting
                measurements (1,:) SolvedSetting.SolvedMeasurement
            end
            import SolvedSetting.SolvedOutcome;
            
            % Get moment matrix
            solvedMM = solvedSetting.SolvedMomentMatrix;
            obj.SolvedMomentMatrix = solvedMM;
            
            % Get marginal measurements
            obj.Marginals = obj.checkAndSortMmts(measurements);
            
            % Get (sorted) measurement indices:
            obj.Indices = uint64.empty(0,2);
            for mmt = obj.Marginals
                obj.Indices(end+1, :) = mmt.Measurement.Index;
            end
            
            % Query for correlation table
            [obj.Correlations, obj.Sequences] = obj.queryOutcomes();
        end
    end
    
    methods(Access=private)
        function sorted = checkAndSortMmts(obj, unsorted)
            arguments
                obj (1,1) SolvedSetting.SolvedJointMeasurement
                unsorted (1,:) SolvedSetting.SolvedMeasurement
            end
            
            % Check number of measurements
            if (length(unsorted) <= 1)
                error("At least two measurements should be supplied.");
            end
            
            % Get party indices
            indices = zeros(size(unsorted));
            for i = 1:length(unsorted)
                indices(i) = unsorted(i).Measurement.Index(1);
            end
            
            % Check for duplicates
            if length(indices) ~= length(unique(indices))
                error("Each measurement must be from a different party.");
            end
            
            % Sort measurements by party
            [~, sortIndex] = sort(indices);
            sorted = SolvedSetting.SolvedMeasurement.empty;
            for i = 1:length(unsorted)
                sorted(end+1) = unsorted(sortIndex(i));
            end
            
        end
        
        function [val, names] = queryOutcomes(obj)
            mm = obj.SolvedMomentMatrix.MomentMatrix;
            
            % Get number of outcomes, per measurement
            dims = uint64(zeros(1, length(obj.Marginals)));
            for mmt_index = 1:length(obj.Marginals)
                mmt = obj.Marginals(mmt_index);
                dims(mmt_index) = length(mmt.Measurement.Outcomes);
            end
            
            % Prepare outputs
            val = zeros(dims);
            names = strings(dims);
            
            % Write retrieved rows into outputs
            for row = mm.MeasurementCoefs(obj.Indices)
                % (assume row.indices are sorted!)
                out_index = num2cell(reshape(row.indices(:, 3),...
                    [1, length(dims)]));
                coefs = row.real_coefficients;
                val(out_index{:}) = coefs * obj.SolvedMomentMatrix.a;
                names(out_index{:}) = row.sequence;
            end
        end
    end
end

