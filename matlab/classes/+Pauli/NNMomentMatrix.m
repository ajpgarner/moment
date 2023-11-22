classdef (InferiorClasses={?MTKMonomial,?MTKPolynomial}) ...
    NNMomentMatrix < MTKMomentMatrix
%NNMOMENTMATRIX Nearest neighbour moment matrix.
    
%% Properties
    properties(GetAccess=public,SetAccess=private)
        Neighbours 
    end
    
    
%% Constructor    
    methods
        function obj = NNMomentMatrix(scenario, level, nn)
            
            % Input validation
            assert(nargin>=1 && isa(scenario, 'PauliScenario'), ...
                "Nearest neighbour moment matrix must be associated with a PauliScenario");
            assert(nargin>=2 && isnumeric(level) && isscalar(level),...
                "Positive integer moment matrix level should be provided.")
            level = uint64(level);
            if nargin < 3
                nn = uint64(0);
            else
                assert(isnumeric(nn) && isscalar(nn))
                nn = uint64(nn);
            end

            
            % Create (or find) matrix from Moment            
            [mm_index, mm_dim] = mtk('moment_matrix', ...
                    scenario.System.RefId, level, ...
                    'neighbours', nn);
            
            obj = obj@MTKMomentMatrix(scenario, level, mm_index, mm_dim);
            obj.Neighbours = nn;      
        end       
    end
end

