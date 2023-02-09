classdef InflationMatrixSystem < MatrixSystem
    %% Constructor
    methods
        function obj = InflationMatrixSystem(infSetting)
            % Superclass c'tor
            obj = obj@MatrixSystem(infSetting);
            
        end
    end
           
    methods(Access=protected)
        function onNewSymbolsAdded(obj)
            arguments
                obj (1,1) Inflation.InflationMatrixSystem
            end
        end
    end 
end



