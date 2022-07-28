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
        function obj = SolvedJointMeasurement(solvedSetting, jointMmt)
            %SOLVEDMEASUREMENT Construct an instance of this class
            arguments
                solvedSetting (1,1) SolvedSetting
                jointMmt (1,1) Setting.JointMeasurement
            end
            
            % Get moment matrix
            solvedMM = solvedSetting.SolvedMomentMatrix;
            obj.SolvedMomentMatrix = solvedMM;
                        
            % Get marginal measurements
            obj.Marginals = obj.getMmts(solvedSetting, jointMmt);
            
            % Get (sorted) measurement indices:
            obj.Indices = jointMmt.Indices;
            
            % Query for correlation table
            [obj.Correlations, obj.Sequences] = obj.queryOutcomes();
        end
    end
    
    methods(Access=private)
        function sorted = getMmts(obj, solvedSetting, mmt)
            arguments
                obj (1,1) SolvedSetting.SolvedJointMeasurement
                solvedSetting (1,1) SolvedSetting
                mmt (1,1) Setting.JointMeasurement
            end
            
            % Check number of measurement
            sorted = SolvedSetting.SolvedMeasurement.empty;
            for i = 1:length(mmt.Marginals)
                sorted(end+1) = solvedSetting.get(mmt.Marginals(i));
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

