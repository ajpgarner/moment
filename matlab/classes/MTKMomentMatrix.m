classdef MTKMomentMatrix < MTKOpMatrix 
%MTKMOMENTMATRIX Moment matrix for scenario.
    
    properties
        Level
    end
    
    methods
        function obj = MTKMomentMatrix(scenario, level)
            %MTKMOMENTMATRIX Construct an instance of this class
            %   Detailed explanation goes here
            
            if nargin < 2
                error("Moment matrix is defined by scenario and level.");
            end
            
            % Create (or find) matrix from Moment
            level = uint64(level);
            [mm_index, mm_dim] = mtk('moment_matrix', ...
                scenario.System.RefId, level);
            
            % Construct as operator matrix object
            obj = obj@MTKOpMatrix(scenario, mm_index, mm_dim, true);
            obj.Level = level;
            
            % Trigger possible notification of symbol generation.
            obj.Scenario.System.UpdateSymbolTable();
        end
    end
end

