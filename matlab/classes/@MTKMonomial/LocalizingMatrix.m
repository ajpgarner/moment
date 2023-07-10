 function val = LocalizingMatrix(obj, level)
% LOCALIZINGMATRIX Create a localizing matrix for this expression.
%
% PARAMS
%   level - The level of matrix to generate. 
%
    
    if ~obj.IsScalar
        error("Localizing matrices can only be created for scalar objects.");
    end
    
    val = MTKLocalizingMatrix(obj.Scenario, level, obj);
    
end