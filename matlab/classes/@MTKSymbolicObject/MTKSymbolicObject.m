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
            is_polynomial = cellfun(@(x) numel(x) > 1, symbol_cell);
            obj.IsPolynomial = any(is_polynomial(:));
        end
    end
    
    %% Named constructors
    methods(Static)
        obj = InitValue(scenario, values);
    end
    
    %% Overriden virtual methods
    methods(Access=protected)
        str = makeObjectName(obj);
        [re, im] = calculateCoefficients(obj)
    end

end
