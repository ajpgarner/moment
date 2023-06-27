 function val = LocalizingMatrix(obj, level)
% LOCALIZINGMATRIX Create a localizing matrix for this expression.
%
% PARAMS
%   level - The level of matrix to generate. 
%           Set to 0 for a 1x1 matrix containing just the monomial 
%           expression.
%
% RETURNS
%   A new OpMatrix.CompositeOperatorMatrix object, containing the
%   localizing matrix associated with this monomial.
%
% See also: OpMatrix.CompositeOperatorMatrix,
%           OpMatrix.LocalizingMatrix
%
    arguments
        obj (1,1) MTKMonomial
        level (1,1) uint64
    end
    % FIXME Move to MTKObject
    
    lm = obj.RawLocalizingMatrix(level);
    val = OpMatrix.CompositeOperatorMatrix(lm, obj.Coefficient);
end