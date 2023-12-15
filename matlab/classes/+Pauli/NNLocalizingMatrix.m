classdef (InferiorClasses={?MTKMonomial,?MTKPolynomial}) ...
        NNLocalizingMatrix < MTKLocalizingMatrix
%NNLOCALIZINGMATRIX Nearest neighbour localizing matrix.
    
%% Properties
    properties(GetAccess=public,SetAccess=private)
        Neighbours
    end
    
%% Constructor    
    methods
        function obj = NNLocalizingMatrix(scenario, level, expr, nn)
            
            % Input validation
            assert(nargin>=1 && isa(scenario, 'PauliScenario'), ...
                "Nearest neighbour moment matrix must be associated with a PauliScenario");
            assert(nargin>=2 && isnumeric(level) && isscalar(level),...
                "Positive integer moment matrix level should be provided.")
            level = uint64(level);
            if nargin < 4
                nn = uint64(0);
            else
                assert(isnumeric(nn) && isscalar(nn))
                nn = uint64(nn);
            end
                        
            % Handle expression
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
            
            % Do Pauli LM creation
            if is_monomial
                [lm_index, lm_dim, actually_mono, is_hermitian] ...
                    = mtk('localizing_matrix', scenario.System.RefId, ...
                          level, monomial_expr, ...
                          'neighbours', nn);
            else
                if expr.FoundAllSymbols && ~scenario.PermitsSymbolAliases
                    [lm_index, lm_dim, actually_mono, is_hermitian] ...
                        = mtk('localizing_matrix', scenario.System.RefId,...
                              level, 'symbols', expr.SymbolCell{1}, ...
                              'neighbours', nn);
                else
                    [lm_index, lm_dim, actually_mono, is_hermitian] ...
                        = mtk('localizing_matrix', scenario.System.RefId,...
                              level, 'operators', expr.OperatorCell{1}, ...
                              'neighbours', nn);
                end                
            end

            % Invoke parent constructor
            obj = obj@MTKLocalizingMatrix(scenario, ...
                {level, lm_index, lm_dim, actually_mono, is_hermitian},...
                expr);
            
            obj.Neighbours = nn;          
        end
    end
end