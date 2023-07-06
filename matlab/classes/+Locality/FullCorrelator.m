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
            if numel(scenario.Parties) ~= 2
                error("Full correlator matrix is only defined between two parties.");
            end
            if ~all(scenario.OutcomesPerMeasurement == 2)
                error("Full correlator matrix is only defined when every measurement has two outcomes.");
            end

            shape = scenario.MeasurementsPerParty + 1;
            obj = obj@MTKPolynomial(scenario, 'overwrite', shape);

            % Special case: 0 measurements
            if obj.IsScalar
                assert(isequal(shape, [1 1]));
                obj.Constituents = MTKMonomial.InitValue(scenario, 1.0);
                return;
            end

            obj.Constituents{1} = MTKMonomial.InitValue(scenario, 1.0);

            for mA_idx = 0:numel(scenario.Parties(1).Measurements)
                for mB_idx = 0:numel(scenario.Parties(2).Measurements)
                    poly_obj = obj.makeCorrObj(mA_idx, mB_idx);
                    obj.Constituents{mA_idx+1, mB_idx+1} = ...
                        poly_obj.Constituents;
                end
            end
        end
    end

    methods(Access=private)
        function val = makeCorrObj(obj, mA, mB)
            partyA = obj.Scenario.Parties(1);
            partyB = obj.Scenario.Parties(2);            
            if mA == 0
                if mB == 0
                    % <I>
                    val = MTKPolynomial.InitValue(obj.Scenario, 1.0);                            
                else
                    % <Bi>
                    impl = partyB.Measurements(mB).ExplicitOutcomes;
                    val = 2*impl - 1;
                end
            elseif mB == 0
                % <Ai>
                impl = partyA.Measurements(mA).ExplicitOutcomes;
                val = 2*impl - 1;
            else
                % <Ai, Bj>
                val = partyA.Measurements(mA).Correlator(...
                    partyB.Measurements(mB));
            end
        end
    end
end
