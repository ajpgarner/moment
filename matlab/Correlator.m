classdef Correlator < handle & RealObject
    %CORR Binary correlator
    %   +1 if outcomes same, -1 if different
    
    properties(SetAccess=private, GetAccess=public)
        Constituents
        Indices
    end
        
    %% Public methods
    methods     
        function obj = Correlator(mmtA, mmtB)
            arguments
                mmtA (1,1) Scenario.Measurement
                mmtB (1,1) Scenario.Measurement
            end
            %CORR Construct an instance of this class

            % Check settings match, and call super class constructor
            if mmtA.Scenario ~= mmtB.Scenario
                error("Measurements should be in same setting.");
            end
            obj = obj@RealObject(mmtA.Scenario);
                
            % Check outcome counts match
            if length(mmtA.Outcomes) ~= length(mmtB.Outcomes)
                error("Measurements should have same number of outcomes.");
            end
            
            % Link constituent parts and indices, in order
            obj.Constituents = Scenario.Measurement.empty;
            if mmtA.Index(1) < mmtB.Index(1)
                obj.Constituents(end+1) = mmtA;
                obj.Constituents(end+1) = mmtB;
                obj.Indices = vertcat(mmtA.Index, ...
                                    mmtB.Index);
            elseif mmtA.Index(1) > mmtB.Index(1)
                obj.Constituents(end+1) = mmtB;
                obj.Constituents(end+1) = mmtA;
                obj.Indices = vertcat(mmtB.Index, ...
                                    mmtA.Index);
            else
                error("Correlators must be between measurements from " ...
                      + "different parties.");
            end
        end
    end
    
    %% Overriden methods
    methods(Access=protected)
        function calculateCoefficients(obj)               
            % Infer number of real elements:
            re_basis_size = size(obj.Constituents(1).Outcomes(1).Coefficients, 2);
            obj.real_coefs = sparse(1, re_basis_size);

            % Build correlators
            for indexA = 1:length(obj.Constituents(1).Outcomes)
                outcomeA = obj.Constituents(1).Outcomes(indexA);
                for indexB = 1:length(obj.Constituents(2).Outcomes)                    
                    outcomeB = obj.Constituents(2).Outcomes(indexB);
                    if indexA == indexB
                        sign = +1;
                    else
                        sign = -1;
                    end
                    outcomeAB = outcomeA * outcomeB;
                    obj.real_coefs = obj.real_coefs + ...
                                     (sign * outcomeAB.Coefficients);
                end
            end
        end
    end
end

