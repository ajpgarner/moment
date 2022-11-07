classdef Monomial < handle
    %MONOMIAL A monomial expression, as part of an algebraic setting
    
    properties(GetAccess = public, SetAccess = protected)
        Setting
        Operators
        Coefficient
        SymbolId = -1
        SymbolConjugated = false
        RealBasisIndex
        ImaginaryBasisIndex
    end
    
    properties(Access=private)
        symbol_id = int64(-1);
        symbol_conjugated = false;
        re_basis_index = -1;
        im_basis_index = -1;
    end
    
    methods
        function obj = Monomial(setting, operators, scale)
            arguments
                setting (1,1) AlgebraicScenario
                operators (1,:) uint64
                scale (1,1) double = 1.0
            end
            obj.Setting = setting;
            obj.Operators = operators;
            obj.Coefficient = scale;
        end
        
    end
    
    %% Accessors: Symbol row info
    methods
        function val = get.SymbolId(obj)
            % Cached value?
            if obj.symbol_id == -1
                obj.loadSymbolInfo();
            end
            
            val = obj.symbol_id;
            
        end
        
        function val = get.SymbolConjugated(obj)
            % Cached value?
            if obj.symbol_id == -1
                obj.loadSymbolInfo();
            end
            
            val = obj.symbol_conjugated;
        end
        
        function val = get.RealBasisIndex(obj)
            % Cached value?
            if obj.symbol_id == -1
                obj.loadSymbolInfo();
            end
            
            val = obj.re_basis_index;
        end
        
        function val = get.ImaginaryBasisIndex(obj)
            % Cached value?
            if obj.symbol_id == -1
                obj.loadSymbolInfo();
            end
            
            val = obj.im_basis_index;
        end
    end
    
    %% Internal methods
    methods (Access=private)
        function success = loadSymbolInfo(obj)
            if obj.Setting.HasMatrixSystem
                [row, conjugated] = npatk('symbol_table', ...
                    obj.Setting.System.RefId, ...
                    obj.Operators);
                if (isa(row, 'logical') && (row == false))
                    obj.setDefaultSymbolInfo();
                    success = false;
                else
                    obj.symbol_id = uint64(row.symbol);
                    obj.re_basis_index = uint64(row.basis_re);
                    obj.im_basis_index = uint64(row.basis_im);
                    obj.symbol_conjugated = logical(conjugated);
                    success = true;
                end
            else
                obj.setDefaultSymbolInfo();
                success = false;
            end
        end
        
        function setDefaultSymbolInfo(obj)
            obj.symbol_id = -1;
            obj.symbol_conjugated = false;
            obj.re_basis_index = 0;
            obj.im_basis_index = 0;
        end
    end
end

