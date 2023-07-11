classdef (InferiorClasses={?MTKMonomial,?MTKPolynomial}) ...
    MTKLocalizingMatrix < MTKOpMatrix
% MTKLOCALIZINGMATRIX Localizing matrix in scenario.

    properties(GetAccess=public, SetAccess=private)
        Level % The level of the matrix.
        Word  % The localizing expression.
    end
    
    
    %% Error message strings
    properties(Constant, Access = protected)
                
        % Error: bad level.
        err_bad_level = ...
            "Localizing matrix level must be a non-negative integer.";
        
        % Error: bad localizing word.
        err_bad_word = ...
            "Localizing matrices can only be created from scalar monomial/polynomial objects.";
    end
    
    %% Constructor
    methods
        function obj = MTKLocalizingMatrix(scenario, level, expr)
            if nargin < 3
                error("Localizing matrix is defined by scenario, level and localizing expression.");
            end
            
            is_monomial = true;
            
            if ~isnumeric(level) || level < 0
                error(MTKLocalizingMatrix.err_bad_level);
            else
                level = uint64(level);
            end
                        
            if isa(expr, "MTKObject")
                disp(expr);
                if ~expr.IsScalar
                    error(MTKLocalizingMatrix.err_bad_word);
                end                
                expr.ReadOnly = true; % TODO: Clone.                
                if isa(expr, "MTKPolynomial")
                    is_monomial = false;
                elseif isa(expr, "MTKMonomial")
                    monomial_expr = uint64(reshape(expr.Operators, 1, []));
                else
                    error(MTKLocalizingMatrix.err_bad_word);
                end
                
            elseif isnumeric(expr)                
                % Treat input as operator string
                monomial_expr = reshape(uint64(expr), 1, []);
                expr = MTKMonomial(scenario, monomial_expr, 1.0);
            end
            
            if is_monomial
                if ~scenario.IsClose(expr.Coefficient, 1)
                    error("Scaled localizing matrix not yet supported.");
                end
                [lm_index, lm_dim, lm_mono, is_hermitian] ...
                    = mtk('localizing_matrix', scenario.System.RefId, ...
                          level, monomial_expr);
                 assert(lm_mono == is_monomial);
            else
                error("Polynomial localizing matrix not yet supported.");
            end
            
            % Construct MTKObject
            obj = obj@MTKOpMatrix(scenario, lm_index, lm_dim, ...
                                  is_monomial, is_hermitian);
            obj.Level = level;
            obj.Word = expr;
            
            % Trigger notification of possible new symbols
            obj.Scenario.System.UpdateSymbolTable();
        end
    end
    
    %% Overriden algebra
    methods
        function val = ctranspose(obj)
            if obj.IsHermitian
                val = obj;
                return
            end
            disp(obj.Word);
            val = MTKLocalizingMatrix(obj.Scenario, obj.Level, ...
                                      ctranspose(obj.Word));
        end
    end
    
    
    %% Overriden methods
    methods(Access=protected)
        function val = getLevel(obj)
            val = obj.Level;
        end
        
        function val = getWord(obj)
            val = obj.Word;
        end
        
        function val = rescaleMatrix(obj, scale)
            val = MTKLocalizingMatrix(obj.Scenario, obj.Level, ...
                                      obj.Word * scale);
        end
    end
end

