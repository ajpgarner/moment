classdef CollinsGisin < MTKMonomial
%COLLINSGISIN Collins-Gisin tensor associated with scenario.
%
% This is a specialization of MTKMonomial with pre-determined properties.
%  
    methods
        function obj = CollinsGisin(scenario)

			if ~isa(scenario, 'LocalityScenario')
				error("Collins-Gisin object only defined"...
					  + " for locality scenario");
			end
          
            shape = scenario.OperatorsPerParty + 1;            
            
            % Set operators, hashes, and coefficients
            obj = obj@MTKMonomial(scenario, 'overwrite', shape);
            [obj.Operators, obj.Hash] = ...
                mtk('collins_gisin', 'sequences', ...
                    obj.Scenario.System.RefId);            
            obj.Coefficient = ones(size(obj));
            obj.ReadOnly = true;
            
        end       
    end
    
    %% Overridden methods
    methods(Access=protected)
        function [id, conj, re, im, aliased] = queryForSymbolInfo(obj)
            
            [id, re] = mtk('collins_gisin', 'symbols', ...
                           obj.Scenario.System.RefId);
            
            dims = size(obj);
            % All operators should be Hermitian....
            conj = false(dims);
            
            % No imaginary parts.
            im = zeros(dims);
            
            % No aliases (in locality scenario, at least!)
            aliased = false(dims);            
        end
        
        
        function str = makeObjectName(obj)
            str = mtk('collins_gisin', 'strings', ...
                      obj.Scenario.System.RefId);
        end
    end
end

