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
 
		if nargin<2 || ~isnumeric(level) || numel(level)~=1
			error("Expected a numeric argument for the level.");
		end
		level = uint64(level);

        if ~obj.IsScalar
            error("Can only generate localizing matrices for scalar polynomials.");
        end
        
        % Call LM constructor.
        val = MTKLocalizingMatrix(obj.Scenario, level, obj);
    end