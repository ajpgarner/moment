classdef CompositeOperatorMatrix < handle
    %COMPOSITEOPERATORMATRIX 
    
    properties(GetAccess=public, SetAccess=private)
        MatrixSystem = MatrixSystem.empty(0,0);
        Constituents
        Weights
        Dimension
        RealBasis
        ImaginaryBasis
    end
        
    %% Construction
    methods
        function obj = CompositeOperatorMatrix(constituents, weights)
            if isa(obj.Constituents, 'OperatorMatrix')
                error("Constituents must be operator matrices.");
            end
            obj.Constituents = reshape(constituents, 1, []);
            obj.Weights = double(reshape(weights, 1, []));  
            
            % Check length of weights matches length of objects
            if length(obj.Constituents) ~= length(obj.Weights)
                error("Number of weights must match number of matrices.");
            end
            
            % Determine dimension, and check all constituents match
            if isempty(obj.Constituents)
                obj.MatrixSystem = MatrixSystem.empty(0,0);
                obj.Dimension = uint64(0);
                obj.RealBasis = sparse(0,0);
                obj.ImaginaryBasis = sparse(0,0);
                
            else
                obj.Dimension = obj.Constituents(1).Dimension;
                obj.MatrixSystem = obj.Constituents(1).MatrixSystem;
                for c = obj.Constituents
                    if obj.Dimension ~= c.Dimension
                        error("Constituent operator matrices must have "...
                            + "the same dimension.");
                    end
                    if obj.MatrixSystem ~= c.MatrixSystem
                       error("Constituent operator matrices must all "...
                            + "be part of the same system.");
                    end
                end
                obj.makeMonolithicBases();
            end
        end
    end
    
    %% CVX Methods
    methods
        function out_M = cvxComplexMatrix(obj, a, b)
            % Get handle to CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            %XXX: Is this M 'global' due to cvx quirks?
            expression M(obj.Dimension, obj.Dimension)
            
            M(:,:) = reshape(transpose(a) * obj.RealBasis ...
                             + transpose(b) * obj.ImaginaryBasis, ...
                        [obj.Dimension, obj.Dimension]);
                        
            % Output handles to cvx objects
            out_M = M;
        end
        
        function out_M = cvxRealMatrix(obj, a)
            % Get handle to CVX problem
            cvx_problem = evalin( 'caller', 'cvx_problem', '[]' );
            if ~isa( cvx_problem, 'cvxprob' )
                error( 'No CVX model exists in this scope!');
            end
            
            %XXX: Is this M 'global' due to cvx quirks?
            expression M(obj.Dimension, obj.Dimension);
            
            M(:,:) = reshape(transpose(a) * obj.RealBasis, ...
                             [obj.Dimension, obj.Dimension]);
            
            % Output handles to cvx objects
            out_M = M;
        end
    end
    
    methods(Access=private)
        function makeMonolithicBases(obj)
            % No basis if no constituents
            if isempty(obj.Constituents) || isempty(obj.MatrixSystem)
                obj.RealBasis = sparse(0,0);
                obj.ImaginaryBasis = sparse(0,0);
                return
            end
            
            % Create empty arrays
            sys = obj.MatrixSystem;           
            obj.RealBasis = sparse(double(sys.RealVarCount), ...
                                   double(obj.Dimension * obj.Dimension));
            obj.ImaginaryBasis = sparse(double(sys.ImaginaryVarCount), ...
                                   double(obj.Dimension * obj.Dimension));
                               
            % Weighted sum over constituents
            for i = 1:length(obj.Constituents)
                cObj = obj.Constituents(i);
                w = obj.Weights(i);
                [re, im] = cObj.SparseMonolithicBasis();
                obj.RealBasis = obj.RealBasis + (w*re);
                obj.ImaginaryBasis = obj.ImaginaryBasis + (w*im);
            end
        end
    end
end
