 function val = times(lhs, rhs)
% MTIMES Elementwise Multiplication.
%
% SYNTAX
%   1. v = polynomial .* double
%   2. v = double .* polynomial 
%   3. v = polynomial .* monomial
%   4. v = monomial .* polynomial 
%   5. v = polynomial_A .* polynomial_B
%
% RETURNS
%   A new MTKPolynomial with appropriate coefficients and operators.
%
% See also: MONOMIAL.TIMES
%

    % Pre-multiplication by a built-in type, or MTKMonomial
    if ~isa(lhs, 'MTKPolynomial')
        this = rhs;
        other = lhs;
        premult = true;
    else
        this = lhs;
        other = rhs;
        premult = false;
    end

    % Check dimensions
    this_size = size(this);
    other_size = size(other);            
    if ~isequal(this_size, other_size)
        if ~(this.IsScalar || numel(other) == 1)
            error("Objects must be same size for _+_, or one must be scalar.");
        end
    end
        
    % If either object is empty, result is empty
    if isempty(this) || isempty(other)
        val = this;
        assert(isempty(val));
        return;
    end
    

    % Handle numerics (commutes!)
    if isnumeric(other)                
        if this.IsScalar 
            val = MTKPolynomial(this.Scenario, other * this.Constituents);
        else                    
            if numel(other)==1
                monos = cellfun(@(x) (other * x), this.Constituents,...
                                'UniformOutput', false);
                val = MTKPolynomial(this.Scenario, monos);
            else
                val = MTKPolynomial(this.Scenario, ...
                      cellfun(@(x,y) (x * y), this.Constituents,...
                              num2cell(other),  'UniformOutput', false));
            end
        end
        return;
    end

    % Handle monomials (including broadcasting)
    if isa(other, 'MTKMonomial')
        if premult
            if this.IsScalar
                if other.IsScalar
                    new_monos = other .* this.Constituents;
                else
                    new_monos = cell(size(other));
                    for idx=1:numel(new_monos)
                        new_monos{idx} = other(idx) .* this.Constituents;
                    end     
                end
            else
                if other.IsScalar
                    new_monos = cellfun(@(x) (other .* x),...
                        this.Constituents, 'UniformOutput', false);
                else
                    new_monos = cell(size(other));
                    for idx=1:numel(new_monos)
                        new_monos{idx} = other(idx) .* this.Constituents{idx};
                    end                            
                end
            end
        else
           if this.IsScalar
                if other.IsScalar
                    new_monos = this.Constituents .* other;
                else
                    new_monos = cell(size(other));
                    for idx=1:numel(new_monos)
                        new_monos{idx} = this.Constituents .* other(idx);
                    end                            
                end
            else
                if other.IsScalar
                    new_monos = cellfun(@(x) (x .* other),...
                        this.Constituents, 'UniformOutput', false);
                else
                    new_monos = cell(size(other));
                    for idx=1:numel(new_monos)
                        new_monos{idx} = this.Constituents{idx} .* other(idx);
                    end                            
                end
            end
        end
        val = MTKPolynomial(this.Scenario, new_monos);
        return;
    end

    % Handle polynomials
    assert(isa(rhs, 'MTKPolynomial'))
    if lhs.IsScalar 
        if rhs.IsScalar
            monomials = ...
                multiplyMonomialVectors(lhs, lhs.Constituents, ...
                                             rhs.Constituents);
        else
            monomials = cell(size(rhs));
            for idx=1:numel(rhs)
                monomials{idx} = ...
                    multiplyMonomialVectors(lhs, lhs.Constituents, ...
                                                 rhs.Constituents{idx});
            end
        end
    elseif rhs.IsScalar
        monomials = cell(size(lhs));
        for idx=1:numel(lhs)
            monomials{idx} = ...
                multiplyMonomialVectors(lhs, lhs.Constituents{idx}, ...
                                             rhs.Constituents);
        end
    else
        assert(isequal(size(lhs), size(rhs)));
        monomials = ...
            cellfun(@(x,y) (multiplyMonomialVectors(lhs, x, y)), ...
                    lhs.Constituents, rhs.Constituents, ...
                    'UniformOutput', false);
    end
    val = MTKPolynomial(lhs.Scenario, monomials);
 end

 %% Private functions
 function val = multiplyMonomialVectors(obj, lhs, rhs)
% MULTIPLY Combine two monomial arrays
    len_lhs = numel(lhs);
    len_rhs = numel(rhs);
    if len_lhs == 0 || len_rhs == 0
        val = MTKMonomial.empty(0, 1);
        return;
    end

    if numel(lhs) == 1
        if numel(rhs) == 1
            opers = [lhs.Operators, rhs.Operators];
            coefs = lhs.Coefficient * rhs.Coefficient;
        else
            opers = cellfun(@(y) [lhs.Operators, y], ...
                            rhs.Operators, ...
                            'UniformOutput', false);
            coefs = lhs.Coefficient .* rhs.Coefficient;
        end                
    elseif numel(rhs) == 1
        opers = cellfun(@(x) [x, rhs.Operators], ...
                        lhs.Operators, ...
                        'UniformOutput', false);
        coefs = lhs.Coefficient .* rhs.Coefficient;
    else
        opers = cellfun(@(x,y) [x, y], ...
                        repmat(lhs.Operators, len_rhs, 1), ...
                        repelem(rhs.Operators, len_lhs, 1), ...
                        'UniformOutput', false);                            
        coefs = repmat(lhs.Coefficient, len_rhs, 1) ...
            .* repelem(rhs.Coefficient, len_lhs, 1);
    end

    % Construct monomials (find canonical form, etc.)
    val = MTKMonomial(obj.Scenario, opers, coefs);

    % Clean-up polynomial
    val = obj.orderAndMerge(val);
end
