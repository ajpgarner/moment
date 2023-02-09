classdef LocalityMatrixSystem < MatrixSystem
    
    properties(SetAccess = private, GetAccess = public)
        ProbabilityTable = struct.empty;
    end
    
    properties(Access = private)
        probability_table = struct.empty;
        max_prob_len = uint64(0);
    end
    
    
    %% Constructor
    methods
        function obj = LocalityMatrixSystem(locSetting)            
            % Superclass c'tor
            obj = obj@MatrixSystem(locSetting);
            
        end
    end
               
    %% Accessors for probability table
    methods
        function val = get.ProbabilityTable(obj)
            % PROBABILITYTABLE A struct-array indicating how each
            %   measurement outcome can be expressed in terms of real
            %   basis elements (including implied probabilities that do not
            %   directly exist as operators in any moment matrix).
            if (isempty(obj.probability_table))
                obj.probability_table = mtk('probability_table', ...
                                              obj.RefId);
            end
            val = obj.probability_table;
        end
        
        function result = MeasurementCoefs(obj, indices)
            arguments
                obj (1,1) Locality.LocalityMatrixSystem
                indices (:,2) uint64
            end            
            parties = indices(:, 1);
            if length(parties) ~= length(unique(parties))
                error("Measurements must be from different parties.");
            end
            
            result = mtk('probability_table', ...
                           obj.RefId, indices);
        end
    end
    
    methods(Access=protected)
        function onNewSymbolsAdded(obj)
            arguments
                obj (1,1) Locality.LocalityMatrixSystem
            end
            obj.ProbabilityTable = mtk('probability_table', obj.RefId);
        end
    end 
end



