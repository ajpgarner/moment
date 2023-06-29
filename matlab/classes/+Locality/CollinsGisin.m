classdef CollinsGisin < MTKMonomial
%COLLINSGISIN Collins-Gisin tensor associated with scenario.
%
% This is a specialization of MTKMonomial with pre-determined properties.
%  
    methods
        function obj = CollinsGisin(scenario)
            arguments
                scenario (1,1) LocalityScenario
            end            
            shape = scenario.OperatorsPerParty + 1;            
            
            % Set operators, hashes, and coefficients
            obj = obj@MTKMonomial(scenario, 'overwrite', shape);
            [obj.Operators, obj.Hash] = ...
                mtk('collins_gisin', 'sequences', ...
                    obj.Scenario.System.RefId);            
            obj.Coefficient = ones(size(obj));
            
        end       
    end
    
    %% Overridden methods
    methods(Access=protected)
        function [id, conj, re, im] = queryForSymbolInfo(obj)
            
            [id, re] = mtk('collins_gisin', 'symbols', ...
                           obj.Scenario.System.RefId);
            
            % All operators should be Hermitian....
            conj = false(size(obj));
            
            % No imaginary parts.
            im = zeros(size(obj));
        end
        
        
        function str = makeObjectName(obj)
            str = mtk('collins_gisin', 'strings', ...
                      obj.Scenario.System.RefId);
        end
    end
end

