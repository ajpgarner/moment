function val = LocalizingMatrix(obj, level)
    % LOCALIZINGMATRIX Create a localizing matrix for this expression.
    %
    % PARAMS
    %   level - The level of matrix to generate. 
    %
    % RETURNS
    %   A new OpMatrix.CompositeOperatorMatrix object, containing one
    %   OpMatrix.LocalizingMatrix for each constituent term of the
    %   monomial, weighted by the appropriate co-efficient.
    %
    % See also: OpMatrix.CompositeOperatorMatrix,
    %           OpMatrix.LocalizingMatrix
    %
        arguments
            obj (1,1) MTKPolynomial
            level (1,1) uint64
        end

        if ~obj.IsScalar
            error("Can only generate localizing matrices for scalar polynomials.");
        end

        lm = OpMatrix.LocalizingMatrix.empty(1,0);
        w = double.empty(1,0);

        for c = obj.Constituents
            lm(end+1) = c.RawLocalizingMatrix(level);
            w(end+1) = c.Coefficient;
        end

        val = OpMatrix.CompositeOperatorMatrix(lm, w);
    end