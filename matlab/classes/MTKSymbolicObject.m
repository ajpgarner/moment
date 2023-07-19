classdef (InferiorClasses={?MTKMonomial, ?MTKPolynomial}) ...
    MTKSymbolicObject < MTKObject
% MTKSYMBOLICOBJECT An MTKObject that can be defined only by its symbols.
%
% Somewhat analagous to Moment::Polynomial in lib_moment.
%
    properties(SetAccess=protected, GetAccess=public)    
        SymbolCell;
        IsPolynomial;
    end

    
    %% Construction
    methods
        function obj = MTKSymbolicObject(scenario, symbol_cell, varargin)
        %MTKSYMBOLICOBJECT Construct an instance of this class.        
           
            dimensions = size(symbol_cell);            
            obj = obj@MTKObject(scenario, dimensions, true);
            
            raw = (numel(varargin) >= 1) && strcmp(varargin{1}, 'raw');
            
            % Put polynomials into canonical form
            if raw 
                obj.SymbolCell = symbol_cell;
            else
                obj.SymbolCell = ...
                    obj.Scenario.SimplifySymbolCell(symbol_cell);
            end
            
            % Determine if object is polynomial
            obj.IsPolynomial = ...
                    any(cellfun(@(x) numel(x) > 1, symbol_cell), 'all');                
        end
    end
    
    %% Named constructors
    methods(Static)
        function obj = InitValue(scenario, values)
            if nargin ~=2 || ~isa(scenario, 'MTKScenario') || ~isnumeric(values)
                error("InitValue takes a scenario, and numeric values as input.");
            end
            
            % Synthesize values into symbol cell
            symbol_cell = cell(size(values));
            for idx=1:numel(values)
                symbol_cell{idx} = {{1, values(idx)}};
            end
            
            % Construct
            obj = MTKSymbolicObject(scenario, symbol_cell, 'raw');
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function str = makeObjectName(obj)
            % Query MTK for canonical object name(s)
            str = mtk('simplify', 'polynomial', 'string_out', ...
                      obj.Scenario.System.RefId, obj.SymbolCell);
        end
    end
       
    
    %% Algebraic manipulations
    methods
        function val = times(lhs, rhs)
            % TIMES Element-wise multiplication .*
            %
            [this, other, ~] = MTKSymbolicObject.find_this(lhs,rhs);
            
            % Only support numeric multiplication
            if ~isnumeric(other)
                error(this.err_undefined_op, "_.*_", class(lhs), class(rhs));
            end
            
            if numel(other) == 1 % Scalar multiplication
                copy_cell = this.SymbolCell;
                for pIdx = 1:numel(copy_cell)
                    for mIdx = 1:numel(copy_cell{pIdx})
                        copy_cell{pIdx}{mIdx}{2} = ...
                            copy_cell{pIdx}{mIdx}{2} * other;
                    end
                end
            elseif numel(this) == 1  % Broadcast over numeric array
                copy_cell = cell(size(other));
                for nIdx = 1:numel(other)
                    copy_cell{nIdx} = this.SymbolCell{1};
                    for mIdx = 1:numel(copy_cell{nIdx})
                        copy_cell{nIdx}{mIdx}{2} = ...
                            copy_cell{nIdx}{mIdx}{2} * other(nIdx);
                    end
                end
            else % Element-wise multiplication
                if ~isequal(size(this), size(other))
                    error("_.*_ is only supported when array sizes match, or one array is scalar.");
                end
                copy_cell = this.SymbolCell;
                for pIdx = 1:numel(copy_cell)
                    for mIdx = 1:numel(copy_cell{pIdx})
                        copy_cell{pIdx}{mIdx}{2} = ...
                            copy_cell{pIdx}{mIdx}{2} * other(pIdx);
                    end
                end
            end
            
            % Make new object from rescaled cell
            val = MTKSymbolicObject(this.Scenario, copy_cell);
        end
        
        function val = mtimes(lhs, rhs)
            % TIMES Matrix multiplication *
            %
            if (numel(lhs) == 1) || (numel(rhs) == 1)
                val = times(lhs, rhs);
            else
                error("_*_ is only supported when one array is scalar."...
                    + " For element-wise multiplication use _.*_");
            end
        end
        
        function val = uminus(obj)
            % UMINUS Unitary minus.
            val = times(obj, -1);
        end
        
        function val = plus(lhs, rhs)
            [this, other, ~] = MTKSymbolicObject.find_this(lhs,rhs);
            
            % Make other a symbolic object (if possible)
            if isnumeric(other)
                other = MTKSymbolicObject.InitValue(scenario, other);
            elseif isa(other, 'MTKPolynomial') || isa(other, 'MTKMonomial')
                other = MTKSymbolicObject(other);
            elseif ~isa(other, 'MTKSymbolicObject')
                error(this.err_undefined_op, "_+_", class(lhs), class(rhs));
            end
            
            if numel(this) == 1 % Broadcast this
                symbol_cell = other.SymbolCell;
                for idx=1:numel(symbol_cell)
                    symbol_cell{idx} = [symbol_cell{idx}, this.SymbolCell{1}];
                end                
            elseif numel(other) == 1 % Broadcast other
                symbol_cell = this.SymbolCell;
                for idx=1:numel(symbol_cell)
                    symbol_cell{idx} = [symbol_cell{idx}, other.SymbolCell{1}];
                end                
            else % Elementwise
                if ~isequal(size(this), size(other))
                    error("_+_ is only supported when object sizes match, or one is scalar.");
                end                
                symbol_cell = this.SymbolCell;
                for idx=1:numel(symbol_cell)
                    symbol_cell{idx} = [symbol_cell{idx}, other.SymbolCell{idx}];
                end                   
            end
            
            % Make new object from combined cells
            val = MTKSymbolicObject(this.Scenario, symbol_cell);                        
        end
        
        function val = minus(lhs, rhs)
            val = plus(lhs, uminus(rhs));
        end
        
    end
    
    %% Private methods
    methods(Static, Access=protected)
        function [this, other, this_on_lhs] = find_this(lhs, rhs)
            if ~isa(lhs, 'MTKSymbolicObject')
                other = lhs;
                this = rhs;
                this_on_lhs = false;
            else
                this = lhs;
                other = rhs;
                this_on_lhs = true;
            end
        end
    end
end