classdef (InferiorClasses={?MTKMonomial,?MTKPolynomial}) ...
    MTKOpMatrix < MTKObject
%MTKOPMATRIX Operator matrix (e.g. moment matrix / localizing matrix.)
%    
    properties(SetAccess = protected, GetAccess = public)
        Index       % The matrix unique ID within a scenario.
        IsMonomial  % True if every element is a monomial.
        IsHermitian % True if matrix is equal to its conjugate tranpose.
    end

    properties(Dependent, GetAccess = public)
        SymbolStrings   % Symbols within matrix (as string).
        SequenceStrings % Operator sequences within matrix (as string).
    end

    properties(Access = private)
        cache_sym_str = string.empty(0,0);
        cache_seq_str = string.empty(0,0);
        cache_mono_rep = MTKMonomial.empty(0,0);
        cache_poly_rep = MTKPolynomial.empty(0,0);
    end

    %% Constructor
    methods
        function obj = MTKOpMatrix(scenario, index, dimension, ...
                                   is_monomial, is_hermitian)
        % MTKOPMATRIX Handle for operator matrices stored in MTK.
        %            
            if (nargin < 1) || ~isa(scenario, 'MTKScenario')
                error('Operator matrix must be associated with a scenario.');
            end
            
            if nargin < 3                                
                error('Matrix index and dimension must be supplied at construction.');
            else
                index = uint64(index);
                assert(isscalar(index));
                
                dimension = uint64(dimension);
                if numel(dimension) == 1
                    dimension = [dimension, dimension];
                else
                    if numel(dimension) ~= 2 || (dimension(1) ~= dimension(2))
                        error("Operator matrix must be square.");
                    end
                end
            end
            
            if nargin < 4
                is_monomial = true;
            end
            
            if nargin < 5
                is_hermitian = false;
            end
            
            % Make MTKObject
            obj = obj@MTKObject(scenario, dimension, true);
            
            % Save matrix-specific properties
            obj.Index = index;
            obj.IsMonomial = logical(is_monomial(1));
            obj.IsHermitian = logical(is_hermitian(1));
        end
    end
    
    %% Convertors
    methods
        function val = MTKMonomial(obj)
            if ~obj.IsMonomial
                error("Cannot cast non-monomial matrix to MTKMonomial.");
            end
            
            if isempty(obj.cache_mono_rep)
                [ops, coefs, hash, symbols, conj, re, im] = ...
                    mtk('operator_matrix', 'monomial', ...
                        obj.Scenario.System.RefId, obj.Index);

                obj.cache_mono_rep = ...
                    MTKMonomial.InitAllInfo(obj.Scenario, ...
                        ops, coefs, hash, symbols, conj, re, im);
                obj.cache_mono_rep.ReadOnly = true;
            end
            val = obj.cache_mono_rep;
        end
        
        function val = MTKPolynomial(obj)
            if isempty(obj.cache_poly_rep)                
                poly_cell = mtk('operator_matrix', 'polynomial', ...
                            obj.Scenario.System.RefId, obj.Index);
                        
                obj.cache_poly_rep = ...
                    MTKPolynomial.InitFromOperatorCell(...
                        obj.Scenario, poly_cell);
                obj.cache_poly_rep.ReadOnly = true;
            end
            
            val = obj.cache_poly_rep;
        end
    end
    
    %% Accessors for matrix element strings
    methods
        function str = get.SymbolStrings(obj)
            if isempty(obj.cache_sym_str)
                obj.cache_sym_str = mtk('operator_matrix', ...
                    'symbol_string', ...
                    obj.Scenario.System.RefId, obj.Index);
            end
            str = obj.cache_sym_str;
        end
    
        function str = get.SequenceStrings(obj)
            if isempty(obj.cache_seq_str)
                obj.cache_seq_str = mtk('operator_matrix', ...
                    'sequence_string', ...
                    obj.Scenario.System.RefId, obj.Index);
            end
            str = obj.cache_seq_str;
        end
    end
    

    %% Virtual method implementations
    methods(Access=protected)
        [re, im] = calculateCoefficients(obj);
        [mask_re, mask_im, elems_re, elems_im] = queryForMasks(obj);
        str = makeObjectName(obj);
    end
    
    %% Virtual method definitions
    methods(Access=protected)
        val = rescaleMatrix(obj, factor);       
        val = getLevel(obj);
        val = getWord(obj);
    end
end

