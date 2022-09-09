classdef OperatorMatrix < handle
    %OPERATORMATRIX Summary of this class goes here
    %   Detailed explanation goes here
        
    properties(SetAccess = protected, GetAccess = public)
        MatrixSystem
        Dimension = uint64(0)
        
        SymbolMatrix
        SequenceMatrix        
        
        RealSymbols
        ImaginarySymbols
        RealSymbolMask
        ImaginarySymbolMask
    end
    
    properties(Access = protected)
        symbol_matrix
        sequence_matrix
    end
    
    methods
        function obj = OperatorMatrix(matrixSystem)
            %OPERATORMATRIX Construct an instance of this class
            arguments
                matrixSystem (1,1) MatrixSystem
            end
            
            obj.MatrixSystem = matrixSystem;
            obj.symbol_matrix = [];
            obj.sequence_matrix = [];
        end
        
        function val = get.SymbolMatrix(obj)
            if isempty(obj.symbol_matrix)
                obj.symbol_matrix = queryForSymbolMatrix(obj);
            end
            val = obj.symbol_matrix;
        end
        
        function val = get.SequenceMatrix(obj)
            if isempty(obj.symbol_matrix)
                obj.symbol_matrix = queryForSequenceMatrix(obj);
            end
            val = obj.symbol_matrix;
        end
        
        function st = SymbolTable(obj)
            st = obj.MatrixSystem.SymbolTable;
        end        
    end
    
    % Virtual methods
    methods(Access = protected)
        function val = queryForSymbolMatrix(obj)
            % Overload this!
            val = [];
        end
        
        function val = queryForSequenceMatrix(obj)
            % Overload this!
            val = [];
        end
    end
end

