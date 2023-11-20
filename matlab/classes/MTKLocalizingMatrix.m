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
        % MTKLOCALIZINGMATRIX Constructs a localizing matrix        
        %
        %   PARAMS:
        %        scenario - The MTKScenario object.
        %        level - The NPA Hierarchy level.
        %        expr - The localizing expression.
        %
            if nargin < 3
                error("Localizing matrix is defined by scenario, level and localizing expression.");
            end
            
            is_monomial = true;
            
            if iscell(level)
                % Construct 'premade' by parent class
                assert(numel(level) == 5, "Invalid level specification.");
                lm_index = level{2};
                lm_dim = level{3};
                actually_monomial = level{4};
                is_hermitian = level{5};
                level = uint64(level{1});
            else
                if ~isnumeric(level) || level < 0
                    error(MTKLocalizingMatrix.err_bad_level);
                else
                    level = uint64(level);
                end

                if isa(expr, "MTKObject")
                    if ~expr.IsScalar
                        error(MTKLocalizingMatrix.err_bad_word);
                    end                                
                    if isa(expr, "MTKPolynomial")
                        expr.ReadOnly = true; 
                        is_monomial = false;
                    elseif isa(expr, "MTKMonomial")
                        if scenario.IsClose(expr.Coefficient, 1)
                            is_monomial = true;
                            monomial_expr = uint64(reshape(expr.Operators, 1, []));
                        else
                            is_monomial = false;
                            expr = MTKPolynomial(expr);
                            expr.ReadOnly = true;
                        end
                    else
                        error(MTKLocalizingMatrix.err_bad_word);
                    end

                elseif isnumeric(expr)                
                    % Treat input as operator string
                    monomial_expr = reshape(uint64(expr), 1, []);
                    expr = MTKMonomial(scenario, monomial_expr, 1.0);
                end

                if is_monomial
                    [lm_index, lm_dim, actually_monomial, is_hermitian] ...
                        = mtk('localizing_matrix', scenario.System.RefId, ...
                              level, monomial_expr);

                else
                    if expr.FoundAllSymbols
                        [lm_index, lm_dim, actually_monomial, is_hermitian]...
                            = mtk('localizing_matrix', scenario.System.RefId,...
                                  level, 'symbols', expr.SymbolCell{1});
                    else
                        [lm_index, lm_dim, actually_monomial, is_hermitian]...
                            = mtk('localizing_matrix', scenario.System.RefId,...
                                  level, 'operators', expr.OperatorCell);
                    end
                end
            end
            
            % Construct MTKObject
            obj = obj@MTKOpMatrix(scenario, lm_index, lm_dim, ...
                                  actually_monomial, is_hermitian);
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

