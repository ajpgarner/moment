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
            
            overwrite = false;
            raw = false;
            read_only = false;
            if numel(varargin) >= 1
                if any(cellfun(@(x) strcmp(x, 'overwrite'), varargin))
                    overwrite = true;
                    raw = true;
                end
                if any(cellfun(@(x) strcmp(x, 'raw'), varargin))
                    raw = true;
                end
                if any(cellfun(@(x) strcmp(x, 'read_only'), varargin))
                    read_only = true;
                end                    
            end
            
            
            dimensions = size(symbol_cell);            
            obj = obj@MTKObject(scenario, dimensions, read_only);
           
            
            % Put polynomials into canonical form
            if raw 
                obj.SymbolCell = symbol_cell;
            else
                obj.SymbolCell = ...
                    obj.Scenario.SimplifySymbolCell(symbol_cell);
            end
            
            % Determine if object is polynomial
            if ~overwrite
                obj.IsPolynomial = test_if_polynomial(obj);
            end
        end
    end
    
    %% Named constructors
    methods(Static)
        obj = InitValue(scenario, values);
        
        function obj = InitForOverwrite(scenario, dimensions)
            obj = MTKSymbolicObject(scenario, ...
                                    cell(dimensions), 'overwrite');
        end
    end
    
    %% Overriden virtual methods
    methods(Access=protected)
        str = makeObjectName(obj);
        
        [re, im] = calculateCoefficients(obj)
        
        mergeIn(obj, merge_dim, offsets, objects);        
        
        mode = spliceIn(obj, indices, value);        
        spliceOut(output, source, indices);
        [output, matched] = spliceProperty(obj, indices, propertyName);
    end

end
