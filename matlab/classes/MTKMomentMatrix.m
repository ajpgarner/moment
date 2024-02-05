classdef (InferiorClasses={?MTKMonomial,?MTKPolynomial}) ...
    MTKMomentMatrix < MTKOpMatrix 
%MTKMOMENTMATRIX Moment matrix for scenario.
    
    properties(GetAccess=public, SetAccess=private)
        Level % The level of the matrix.
    end
    
    methods
        function obj = MTKMomentMatrix(scenario, level, mm_index, mm_dim)
            %MTKMOMENTMATRIX Construct an instance of this class
            %   Detailed explanation goes here
            
            if nargin < 2
                error("Moment matrix is defined by scenario and level.");
            end
            level = uint64(level);
            
            if nargin >= 3
                assert(nargin >= 4, ...
                    "If index supplied, dimension must be supplied");
                assert(isnumeric(mm_index) && isscalar(mm_index), ...
                    "Index must be scalar number.");
                assert(isnumeric(mm_dim) && isscalar(mm_dim), ...
                    "Dimension must be scalar number.");
                
                mm_index = uint64(mm_index);
                mm_dim = uint64(mm_dim);
            else
                % Create (or find) matrix from Moment            
                [mm_index, mm_dim] = mtk('moment_matrix', ...
                    scenario.System.RefId, level);    
            end
            
            
            
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
            val = MTKMonomial.InitValue(obj.Scenario, 1.0); % I
        end
        
        function val = rescaleMatrix(obj, scale)
            val = MTKLocalizingMatrix(obj.Scenario, obj.Level, ...
                    MTKMonomial.InitValue(scale));
        end
    end
end

