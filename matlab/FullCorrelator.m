classdef FullCorrelator < handle
    %FULLCORRELATOR Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(SetAccess={?FullCorrelator, ?Scenario}, GetAccess=public)
        Scenario
        Shape
    end
    
    methods
        function obj = FullCorrelator(scenario)
            arguments
                scenario (1,1) Scenario
            end
            obj.Scenario = scenario;
            obj.Shape = obj.Scenario.MeasurementsPerParty + 1;
        end
        
        function val = linfunc(obj, tensor)
            if (length(size(tensor)) ~= length(obj.Shape)) ...
                || any(size(tensor) ~= obj.Shape)
                error("Expected tensor with dimension " + ...
                    mat2str(obj.Shape));
            end
            
            % TODO: Require moment matrix...
            if ~obj.Scenario.HasMomentMatrix
                error("Must generate MomentMatrix for scenario first.");
            end
            
            dimension = length(size(obj.Shape));
            total_size = prod(obj.Shape);
            
            real_coefs = sparse(zeros(1, length(obj.Scenario.Normalization.Coefficients)));
            
            for i = 1:total_size
                % Skip zeros
                if tensor(i) == 0
                    continue
                end
                
                indices = Util.index_to_sub(obj.Shape, i);
                coef = tensor(i);
                rObj = obj.at(indices - 1);
                real_coefs = real_coefs + sparse(coef * rObj.Coefficients);
            end
            val = RealObject(obj.Scenario, real_coefs);
        end
        
        function val = at(obj, index)
            arguments
                obj (1,1) FullCorrelator
                index (1,:) uint64
            end
            if length(index) ~= length(obj.Shape)
                error("Must supply " + length(obj.Shape) + " indices.");
            end
            if any(index >= obj.Shape)
                error("Index for party " + (find(index >= obj.Shape, 1)) ...
                      + " is out of bounds.");
            end
            
            % Build get command...
            mmt_count = nnz(index);
            get_index = uint64(zeros(mmt_count, 2));
            
            write_index = 1;
            for party_index = 1:length(obj.Shape)
                if index(party_index) > 0
                    get_index(write_index, :) = [party_index, index(party_index)];
                    write_index = write_index + 1;
                end
            end
            
            val = obj.Scenario.get(get_index);
        end
    end
end

