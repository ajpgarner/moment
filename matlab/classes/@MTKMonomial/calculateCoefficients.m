 function [re, im] = calculateCoefficients(obj)

    % Early exit if we can't get symbol information...
    if ~all(obj.SymbolId >= 0, 'all')
        error("Symbols are not yet all registered in matrix system.");
    end 

    % Real coefficients
    switch obj.DimensionType
        case 0 % SCALAR                    
            sys = obj.Scenario.System;
            re = zeros(double(sys.RealVarCount), 1, 'like' sparse(1i));
            if obj.re_basis_index > 0
                re(obj.re_basis_index) = obj.Coefficient;
            end
        case 1
            re = makeColOfRealCoefficientsRV(obj);
        case 2 % ROW- and COL-VECTOR
            re = makeColOfRealCoefficients(obj, 1);
        otherwise                    
            dims = size(obj);
            if length(dims) <= 2
                re = cell(dims(2), 1);
            else
                celldims = cell(dims);
                re = cell(celldims{2:end});
            end

            % Iterate over remaining dimensions
            for idx = 1:prod(dims(2:end))
                re{idx} = makeColOfRealCoefficients(obj, idx);
            end
    end

    % Imaginary coefficients
    switch obj.DimensionType
        case 0 % SCALAR
            sys = obj.Scenario.System;
			im = zeros(double(sys.ImaginaryVarCount), 1, 'like' sparse(1i));
            if obj.im_basis_index > 0
                if obj.SymbolConjugated
                    im(obj.im_basis_index) = -1i * obj.Coefficient;
                else
                    im(obj.im_basis_index) = 1i * obj.Coefficient;
                end
            end
        case 1
            im = makeColOfImaginaryCoefficientsRV(obj);
        case 2 % ROW- and COL-VECTOR
            im = makeColOfImaginaryCoefficients(obj, 1);
        otherwise
             dims = size(obj);
            if length(dims) <= 2
                im = cell(dims(2), 1);
            else
                celldims = cell(dims);
                im = cell(celldims{2:end});
            end

            % Iterate over remaining dimensions
            for idx = 1:prod(dims(2:end))
                im{idx} = makeColOfImaginaryCoefficients(obj, idx);
            end
    end
 end

%% Private functions
function val = makeColOfImaginaryCoefficients(obj, index)
    im_var_count = double(obj.Scenario.System.ImaginaryVarCount);
    num_cols = size(obj, 1);

    imbi = double(reshape(obj.im_basis_index(:, index), 1, []));
    imbi_mask = imbi > 0;
	val = zeros(im_var_count, num_cols, 'like', sparse(1i));
    idx = im_var_count * (0:(num_cols-1)) + imbi;
    idx = idx(imbi_mask);

    values = (1i * ~obj.symbol_conjugated(imbi_mask)) + ...
        -1i * obj.symbol_conjugated(imbi_mask);
    values = values .* obj.Coefficient(imbi_mask, index);
    val(idx) = values;
end

function val = makeColOfImaginaryCoefficientsRV(obj)
    im_var_count = double(obj.Scenario.System.ImaginaryVarCount);
    num_cols = size(obj, 2);

    imbi = double(obj.im_basis_index);
    imbi_mask = imbi > 0;
	val = zeros(im_var_count, num_cols, 'like', sparse(1i));
    idx = im_var_count * (0:(num_cols-1)) + imbi;
    idx = idx(imbi_mask);

    values = (1i * ~obj.symbol_conjugated(imbi_mask)) + ...
        -1i * obj.symbol_conjugated(imbi_mask);
    values = values .* obj.Coefficient(imbi_mask);
    val(idx) = values;
end

function val = makeColOfRealCoefficients(obj, index)
    re_var_count = double(obj.Scenario.System.RealVarCount);
    num_cols = size(obj, 1);

    rebi = double(reshape(obj.re_basis_index(:, index), 1, []));	
    rebi_mask = rebi > 0;

	val = zeros(re_var_count, num_cols, 'like', sparse(1i));
    idx = re_var_count * (0:(num_cols-1)) + rebi;
    idx = idx(rebi_mask);

    val(idx) = obj.Coefficient(rebi_mask, index);
end

function val = makeColOfRealCoefficientsRV(obj)
    re_var_count = double(obj.Scenario.System.RealVarCount);
    num_cols = size(obj, 2);

    rebi = double(obj.re_basis_index);
    rebi_mask = rebi > 0;

    val = zeros(re_var_count, num_cols, 'like', sparse(1i));
    idx = re_var_count * (0:(num_cols-1)) + rebi;
    idx = idx(rebi_mask);

    val(idx) = obj.Coefficient(rebi_mask);
end