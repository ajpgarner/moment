classdef CompositeOperatorMatrix < handle
    %COMPOSITEOPERATORMATRIX 
    
    properties(GetAccess=public, SetAccess=private)
        MatrixSystem = MatrixSystem.empty(0,0);
        Constituents
        Weights
        Dimension
    end
    
    properties(Dependent, GetAccess=public)
        RealBasis
        ImaginaryBasis
        RealBasisElements
        ImaginaryBasisElements
        RealMask
        ImaginaryMask
    end 
    
    properties(Access = protected)
        real_basis
        im_basis
        real_basis_elems = uint64.empty(1,0);
        im_basis_elems = uint64.empty(1,0);
    end
        
    %% Construction
    methods
        function obj = CompositeOperatorMatrix(constituents, weights)
            if isa(obj.Constituents, 'OpMatrix.OperatorMatrix')
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
                obj.real_basis = sparse(0,0);
                obj.im_basis = sparse(0,0);
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
    
    %% Basis accessors
    methods
        function val = get.RealBasis(obj)
            % Resize real if necessary
            delta_re = obj.MatrixSystem.RealVarCount ...
                        - size(obj.real_basis, 2);
                    
            if delta_re > 0               
               obj.real_basis = ...
                   [obj.real_basis, ...
                    sparse(double(obj.Dimension * obj.Dimension), ...
                           double(delta_re))];               
            end
            
            
            % Otherwise, just get basis
            val = obj.real_basis;
        end
        
        function val = get.ImaginaryBasis(obj)
            % Resize imaginary if necessary
            delta_im = obj.MatrixSystem.ImaginaryVarCount ...
                        - size(obj.im_basis, 2);
            if delta_im > 0
               obj.im_basis = ...
                   [obj.im_basis, ...
                    sparse(double(obj.Dimension * obj.Dimension), ...
                           double(delta_im))];                                                    
            end
            
            val = obj.im_basis;
        end 
    end
    
    %% Mask accessors
    methods
        function val = get.RealBasisElements(obj)
            if isempty(obj.real_basis_elems)
                obj.real_basis_elems = uint64.empty(1,0);
                for c = obj.Constituents
                    obj.real_basis_elems = ...
                        [obj.real_basis_elems, c.RealBasisElements];
                end
                obj.real_basis_elems = unique(obj.real_basis_elems, ...
                                              'sorted');                
            end
            val = obj.real_basis_elems;
        end
        
        function val = get.ImaginaryBasisElements(obj)
            if isempty(obj.im_basis_elems)
                obj.im_basis_elems = uint64.empty(1,0);
                for c = obj.Constituents
                    obj.im_basis_elems = ...
                        [obj.im_basis_elems, c.ImaginaryBasisElements];
                end
                obj.im_basis_elems = unique(obj.im_basis_elems, ...
                                              'sorted');                
            end
            val = obj.im_basis_elems;
        end
        
        function val = get.RealMask(obj)
            val = false(1, obj.MatrixSystem.RealVarCount);
            for c = obj.Constituents
                val = val | c.RealMask;
            end
        end
        
        function val = get.ImaginaryMask(obj)
            val = false(1, obj.MatrixSystem.ImaginaryVarCount);
            for c = obj.Constituents
                val = val | c.ImaginaryMask;
            end
        end
    end
    
    %% Apply values
    methods        
        function val = ApplyRules(obj, rulebook )
        % APPLYRULES Transform moments of matrix according to rulebook.
        %
        % Effectively applies rules to each constituent matrix in turn.
        % 
            arguments
                obj (1,1) OpMatrix.CompositeOperatorMatrix
                rulebook (1,1) MomentRuleBook
            end
            tx_monos = OpMatrix.OperatorMatrix.empty(1,0);
            for idx = 1:length(obj.Constituents)
                tx_monos(end+1) = obj.Constituents(idx).ApplyRules(rulebook);
            end
            
            val = OpMatrix.CompositeOperatorMatrix(tx_monos, obj.Weights);
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
            
            M(:,:) = reshape(obj.RealBasis * a...
                             + obj.ImaginaryBasis * b, ...
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
            
            M(:,:) = reshape(obj.RealBasis * a, ...
                             [obj.Dimension, obj.Dimension]);
            
            % Output handles to cvx objects
            out_M = M;
        end
    end
    
    %% YALMIP methods
    methods
        function out_M = yalmipComplexMatrix(obj, a, b)
            out_M = reshape(obj.RealBasis * a ...
                + obj.ImaginaryBasis * b, ...
                [obj.Dimension, obj.Dimension]);
        end
        
        function out_M = yalmipRealMatrix(obj, a)            
            out_M = reshape(obj.RealBasis * a, ...
                            [obj.Dimension, obj.Dimension]);
        end
    end
    
    %% Internal methods
    methods(Access=private)
        function makeMonolithicBases(obj)
            % No basis if no constituents
            if isempty(obj.Constituents) || isempty(obj.MatrixSystem)
                obj.real_basis = sparse(0,0);
                obj.im_basis = sparse(0,0);
                return
            end
            
            % Create empty arrays
            sys = obj.MatrixSystem;           
            obj.real_basis = sparse(double(obj.Dimension * obj.Dimension),...
                                    double(sys.RealVarCount));
            obj.im_basis = sparse(double(obj.Dimension * obj.Dimension),...
                                  double(sys.ImaginaryVarCount));

            % First trigger all constituents to make bases
            for i = 1:length(obj.Constituents)
                [~,~] = obj.Constituents(i).SparseMonolithicBasis();
            end
            
            % Now take weighted sum over constituent bases
            for i = 1:length(obj.Constituents)
                cObj = obj.Constituents(i);
                w = obj.Weights(i);
                [re, im] = cObj.SparseMonolithicBasis();
                obj.real_basis = obj.real_basis + (w*re);
                obj.im_basis = obj.im_basis + (w*im);
            end
        end
    end
end
