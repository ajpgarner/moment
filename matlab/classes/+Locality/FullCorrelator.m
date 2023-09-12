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
            if ~isa(scenario, 'LocalityScenario')
				error("Full correlator object only defined"...
					  + " for locality scenario");
			end
			
            % Check scenario admits a full correlator
            if ~all(scenario.OutcomesPerMeasurement == 2)
                error("Full correlator matrix is only defined when every measurement has two outcomes.");
            end
            
            
            % Query for data
            poly_spec = mtk('full_correlator', scenario.System.RefId, ...
                          'full_sequences');
            obj = obj@MTKPolynomial(scenario, poly_spec, 'direct');
            obj.ReadOnly = true;
        end
    end
end
