 function val = RawLocalizingMatrix(obj, level)
    % RAWLOCALIZINGMATRIX Create a normalized localizing matrix for this expression.
    %
    % The created matrix ignores the coefficient associated with the
    % monomial (i.e. just creating a LM for the operator 'word'). To 
    % construct the full localizing matrix taking into account the
    % co-efficient, use Monomial.LocalizingMatrix instead.
    %
    % PARAMS
    %   level - The level of matrix to generate. 
    %           Set to 0 for a 1x1 matrix containing just the monomial 
    %           expression.
    %
    % RETURNS
    %   A new OpMatrix.LocalizingMatrix object.
    %
    % See also: Monomial.LocalizingMatrix, OpMatrix.LocalizingMatrix
    %
    arguments
        obj (1,1) MTKMonomial
        level (1,1) uint64
    end

    % FIXME Move to MTKObject
    val = OpMatrix.LocalizingMatrix(obj.Scenario, ...
        obj.Operators, level);
end