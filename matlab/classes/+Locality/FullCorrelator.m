classdef FullCorrelator < MTKPolynomial
%FULLCORRELATOR Full correlator tensor associated with scenario
%
% This is a specialization of MTKPolynomial with pre-determined properties.
%  

    
    properties(Access=private)
        mono_coefs = double.empty
    end
    
    methods
        function obj = FullCorrelator(scenario)
            arguments
                scenario (1,1) LocalityScenario
            end
            
            % Check scenario admits a full correlator
            if ~all(scenario.OutcomesPerMeasurement == 2)
                error("Full correlator tensor is only defined when every measurement has binary outcomes.");
            end            
            shape = scenario.MeasurementsPerParty + 1;
            
            obj = obj@MTKPolynomial(scenario, 'overwrite', shape);
            
            
        end
        
%         function val = get.Coefficients(obj)
%             % Silently fail if no moment matrix yet
%             if ~obj.Scenario.HasMatrixSystem
%                 %TODO: Check deep enough matrix system
%                 val = double.empty;
%                 return;
%             end
%             
%             % Return cached value, if any
%             if ~isempty(obj.mono_coefs)
%                 val = obj.mono_coefs;
%                 return;
%             end
%             
%             % Build monolith of co-efficients
%             coefs = length(obj.Scenario.Normalization.Coefficients);
%             elems = prod(obj.Shape);
%             
%             global_i = double.empty(1,0);
%             global_j = double.empty(1,0);
%             global_v = double.empty(1,0);
%             
%             for index = 1:elems
%                 indices = Util.index_to_sub(obj.Shape, index) - 1;
%                 thing = obj.at(indices);
%                 [~, coefs_j, coefs_val] = find(thing.Coefficients);
%                 global_i = horzcat(global_i, ...
%                                 ones(1, length(coefs_j))*index);
%                 global_j = horzcat(global_j, coefs_j);
%                 global_v = horzcat(global_v, coefs_val);
%                 
%             end
%             
%             % Cache and return
%             obj.mono_coefs = sparse(global_i, global_j, global_v, ...
%                                     elems, coefs);
%             val = obj.mono_coefs;            
%         end
%        
    end
end

