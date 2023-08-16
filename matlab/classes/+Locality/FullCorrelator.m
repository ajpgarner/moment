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

            shape = scenario.MeasurementsPerParty + 1;
            obj = obj@MTKPolynomial(scenario, 'overwrite', shape);
            obj.ReadOnly = true;

            % Special case: 0 measurements
            if obj.IsScalar
                assert(isequal(shape, [1 1]));
                obj.Constituents = MTKMonomial.InitValue(scenario, 1.0);
                return;
            end
            
            % Multi-variable iterator
            for offset = 1:prod(shape)
                mmts = MTKUtil.index_to_sub(shape, offset);
                party_mask = mmts > 1;
                mmts = mmts(party_mask) - 1;
                parties = find(party_mask);
                obj.Constituents{offset} = obj.makeCorrelator(parties, mmts);
            end
        end
    end

    methods(Access=private)
        function val = makeCorrelator(obj, party_idx, mmt_idx)
            % Special case, no parties -> <I>
            if numel(party_idx) == 0
                val = MTKMonomial.InitValue(obj.Scenario, 1.0);  
                return;
            end
            
            % Otherwise, get measurements
            ops = cell(numel(party_idx), 1);
            for i = 1:numel(party_idx)
                party = obj.Scenario.Parties(party_idx(i));
                mmt = party.Measurements(mmt_idx);
                ops{i} = mmt.ExplicitOutcomes;
            end

            % Start with expectation value of 1st measurement
            expt = 2*ops{1} - 1;
            
            % Then, multiply at an operator level...
            for j=2:numel(party_idx)
                expt = expt * (2*ops{j} - 1);
            end
            
            % Get constituents
            val = expt.Constituents;            
        end
    end
end
