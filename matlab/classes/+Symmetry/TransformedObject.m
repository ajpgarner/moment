classdef (InferiorClasses={?MTKMonomial, ?MTKPolynomial}) ...
    TransformedObject < MTKSymbolicObject
%TRANSFORMEDOBJECT A symmetrized variant of supplied input object.
%
% Note: if manipulated, can easily degrade into MTKSymbolicObject.
%

   %% Members
   properties(SetAccess=private,GetAccess=public)
       BaseObject
   end

   %% Construction
    methods
        function obj = TransformedObject(scenario, base_object)
        %DERIVEDOBJECT Construct an instance of this class.        
           
            if nargin < 1 || ~isa(scenario, 'SymmetrizedScenario')
                error("First argument must be a symmetrized scenario.");
            end
            if nargin < 2 || ~isa(base_object, 'MTKObject')
                error("Second argument must be an MTKObject");
            end
                        
            % Infer symbol cell of base
            if isa(base_object, 'MTKPolynomial')
                cell_input = base_object.SymbolCell;                
            elseif isa(base_object, 'MTKMonomial')
                cell_input = base_object.SymbolCell;
            else
                error("Cannot transform object of type %s", class(base_object));
            end
            
            % Transform object
            transformed_cell = mtk('transform_symbols', ...
                                   'output', 'symbols', ...
                                   scenario.System.RefId, cell_input);
 
            % Call parent constructor
            obj = obj@MTKSymbolicObject(scenario, transformed_cell, ...
                                        'raw', 'read_only');
            obj.BaseObject = base_object;            
        end
    end
    
    %% Overriden methods
    methods(Access=protected)
        function result = isPropertyMTKObject(obj, property_name)
            switch property_name
                case 'BaseObject'
                    result = true;
                otherwise
                    result = ...
                        isPropertyMTKObject@MTKSymbolicObject(obj, ...
                                                            property_name);
            end
        end
    end
end