classdef MTKValueMatrix < MTKOpMatrix
%MTKVALUEMATRIX MTK Matrix object, representing only numerical values
       
    methods
        function obj = MTKValueMatrix(scenario, values, label)
             
            % Validate input...
            assert(nargin >= 2, ...
                "Value matrix is defined by scenario and numerical data.");
              
            extra_params = cell(1,0);
            if nargin >= 3
                extra_params = {'label', label};
            end
            
            % Create matrix in Moment...
            [mm_index, mm_dim, is_monomial, is_hermitian] = ...
                mtk('value_matrix', scenario.System.RefId, values, ...
                    extra_params{:});           
                    
            assert(is_monomial, "Value matrix unexpectedly non-monomial");
                        
            % Construct as operator matrix objectmt
            obj = obj@MTKOpMatrix(scenario, mm_index, mm_dim, ...
                                  true, is_hermitian);
            
            % Trigger possible notification of symbol generation.
            obj.Scenario.System.UpdateSymbolTable();
        end       
    end
    
    %% Overriden methods
    methods(Access=protected)
        function val = getLevel(obj)
            throw(MException('mtk:no_level', ...
                             "Matrix does not define a level.")); 
        end
        
        function val = getWord(obj)
            throw(MException('mtk:no_word', ...
                             "Matrix does not define a localizing word."));
        end
        
        function val = rescaleMatrix(obj, scale)
            assert(isnumeric(scale) && isscalar(scale), ...
                   "Rescaling must be by scalar factor.");
            [id, dim, is_mono, is_herm] = ...
                mtk('multiply', obj.Scenario.System.RefId, ...
                    obj.Index, scale);
            val = MTKOpMatrix(obj.Scenario, id, dim, is_mono, is_herm);
        end
    end
end

