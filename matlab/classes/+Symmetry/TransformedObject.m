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
                if base_object.IsScalar
                    cell_input = {base_object.SymbolCell};
                else
                    cell_input = base_object.SymbolCell;
                end                                
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
            obj = obj@MTKSymbolicObject(scenario, transformed_cell, 'raw');
            obj.BaseObject = base_object;            
        end
    end
    
end