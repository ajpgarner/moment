classdef (InferiorClasses={?MTKMonomial,?MTKPolynomial}) ...
    NNMomentMatrix < MTKMomentMatrix
%NNMOMENTMATRIX Nearest neighbour moment matrix.
    
    properties(GetAccess=public,SetAccess=private)
        Neighbours
        Wrap        
    end
    
    methods
        function obj = NNMomentMatrix(scenario, level, nn, wrap)
            
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
            if nargin < 4
                wrap = false;
            else
                assert(isscalar(wrap));
                wrap = logical(wrap);
            end
            
            % Create (or find) matrix from Moment            
            [mm_index, mm_dim] = mtk('moment_matrix', ...
                    scenario.System.RefId, level, ...
                    'neighbours', nn, 'wrap', wrap);
            
            obj = obj@MTKMomentMatrix(scenario, level, mm_index, mm_dim);
            obj.Neighbours = nn;
            obj.Wrap = wrap;            
        end       
    end
end

