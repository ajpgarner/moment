classdef (InferiorClasses={?MTKMonomial,?MTKPolynomial}) ...
    MTKMomentMatrix < MTKOpMatrix 
%MTKMOMENTMATRIX Moment matrix for scenario.
    
    properties(GetAccess=public, SetAccess=private)
        Level % The level of the matrix.
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
            % NB: MM are monomial and hermitian by construction.
            obj = obj@MTKOpMatrix(scenario, mm_index, mm_dim, true, true);
            obj.Level = level;
            
            % Trigger possible notification of symbol generation.
            obj.Scenario.System.UpdateSymbolTable();
        end
    end
    
    %% Overriden methods
    methods(Access=protected)
        function val = getLevel(obj)
            val = obj.Level;
        end
        
        function val = getWord(obj)
            val = MTKMonomial.InitValue(1.0); % I
        end
        
        function val = rescaleMatrix(obj, scale)
            val = MTKLocalizingMatrix(obj.Scenario, obj.Level, ...
                    MTKMonomial.InitValue(scale));
        end
    end
end

