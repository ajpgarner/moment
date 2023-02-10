classdef FullCorrelator < handle
    %FULLCORRELATOR
    
    properties(SetAccess={?FullCorrelator, ?Scenario}, GetAccess=public)
        Scenario
        Shape
        Coefficients
    end
    
    properties(Access=private)
        mono_coefs = double.empty
    end
    
    methods
        function obj = FullCorrelator(scenario)
            arguments
                scenario (1,1) LocalityScenario
            end
            obj.Scenario = scenario;
            obj.Shape = obj.Scenario.MeasurementsPerParty + 1;
        end
        
        function val = get.Coefficients(obj)
            % Silently fail if no moment matrix yet
            if ~obj.Scenario.HasMatrixSystem
                %TODO: Check deep enough matrix system
                val = double.empty;
                return;
            end
            
            % Return cached value, if any
            if ~isempty(obj.mono_coefs)
                val = obj.mono_coefs;
                return;
            end
            
            % Build monolith of co-efficients
            coefs = length(obj.Scenario.Normalization.Coefficients);
            elems = prod(obj.Shape);
            
            global_i = double.empty(1,0);
            global_j = double.empty(1,0);
            global_v = double.empty(1,0);
            
            for index = 1:elems
                indices = Util.index_to_sub(obj.Shape, index) - 1;
                thing = obj.at(indices);
                [~, coefs_j, coefs_val] = find(thing.Coefficients);
                global_i = horzcat(global_i, ...
                                ones(1, length(coefs_j))*index);
                global_j = horzcat(global_j, coefs_j);
                global_v = horzcat(global_v, coefs_val);
                
            end
            
            % Cache and return
            obj.mono_coefs = sparse(global_i, global_j, global_v, ...
                                    elems, coefs);
            val = obj.mono_coefs;            
        end
        
        function val = linfunc(obj, tensor)
            arguments
                obj (1,1) Locality.FullCorrelator
                tensor double
            end
            if (length(size(tensor)) ~= length(obj.Shape)) ...
                || any(size(tensor) ~= obj.Shape)
                error("Expected tensor with dimension " + ...
                    mat2str(obj.Shape));
            end
            
            % Require moment matrix...
            if ~obj.Scenario.HasMatrixSystem
                error("Must generate MatrixSystem for scenario first.");
            end
            
            % Calculate coefficients as a monolith
            full_coefs = obj.Coefficients;
            
            total_size = prod(obj.Shape);
            reshape_tensor = sparse(reshape(tensor, [1, total_size]));
            
            real_coefs = reshape_tensor * full_coefs;
            val = Abstract.RealObject(obj.Scenario, real_coefs);
        end
        
        function val = at(obj, index)
            arguments
                obj (1,1) Locality.FullCorrelator
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

