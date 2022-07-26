classdef Correlator < handle
    %CORR Binary correlator
    %   +1 if outcomes same, -1 if different
    
    properties(SetAccess=private, GetAccess=public)
        Constituents
        Indices
        Coefficients
        Setting
    end
    
    properties(Access=private)
        real_coefs = double.empty
    end
    
    methods
        function obj = Correlator(mmtA, mmtB)
            arguments
                mmtA (1,1) Measurement
                mmtB (1,1) Measurement
            end
            %CORR Construct an instance of this class
    
            if length(mmtA.Outcomes) ~= length(mmtB.Outcomes)
                error("Measurements should have same number of outcomes.");
            end
            
            if mmtA.Setting ~= mmtB.Setting
                error("Measurements should be in same setting.");
            end
            obj.Setting = mmtA.Setting;
            
            obj.Constituents = Measurement.empty;
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
        
        function val = get.Coefficients(obj)
            if isempty(obj.real_coefs)
                % (Silently) do nothing if no moment matrix
                if ~obj.Setting.HasMomentMatrix
                    return
                end
               
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
            
            val = obj.real_coefs;
        end
        
        function val= mtimes(obj, rhs)            
            coefs = obj.Coeefficients;
            if isempty(coefs)
                error("Could not calculate coefficients for this "...
                      + "correlator. Perhaps a MomentMatrix has "...
                      + "not yet been generated for this setting?");
            end
            val = coefs * real_basis;
        end
    end
    
    %% CVX Methods
    methods
        function cvx_expr = cvx(obj, real_basis)
            arguments
                obj (1,1) Correlator
                real_basis (:, 1)
            end
            % Get coefficients
            coefs = obj.Coefficients;
            % NB: Zero sparse array is not empty...!
            if isempty(coefs)
                error("Could not calculate coefficients for this "...
                      + "correlator. Perhaps a MomentMatrix has not "...
                      + "yet been generated for this setting?");
            end
            
            % real_basis should be cvx object
            if ~isa(real_basis, 'cvx')
                error("Expected CVX real basis vector input.");
            end 
            
            if length(coefs) ~= length(real_basis)
                error("CVX real basis vector dimension does not match "...
                      + "Correlator coefficient dimension.");
            end
            
            % Generate expression...
            cvx_expr = coefs * real_basis;
        end 
    end
end

