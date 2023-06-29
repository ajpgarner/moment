 function val = plus(lhs, rhs)
% PLUS Addition.
%
% SYNTAX:
%   1. val = mono + double
%   2. val = double + mono
%   3. val = mono_A + mono_B
%
% RETURNS (Syntax 1,2)
%   0 if monomial is proportional to identity with coefficient of
%   -double. MTKMonomial, if monomial is proportional to 
%   identity but not equal to double. MTKPolynomial with two 
%   terms otherwise.
%
% RETURNS (Suntax 3)
%   MTKMonomial if operator string in two monomials matches,
%   and the coefficients are not negations of each other. 0 if the
%   operator string of the two monomials matches, and the
%   coefficients negate each other. MTKPolynomial with two 
%   terms otherwise.
%   
% Due to class precedence, "poly + mono" and "mono + poly" cases 
% are handled by MTKPolynomial.plus.
%
% See also: ALGEBRAIC.POLYNOMIAL.PLUS
%
    % Add a scalar by a built-in type?
    if ~isa(lhs, 'MTKMonomial')
        this = rhs;
        other = lhs;
    else
        this = lhs;
        other = rhs;
    end

    % Check dimensions
    this_size = size(this);
    other_size = size(other);            
    if ~isequal(this_size, other_size)
        if ~(this.IsScalar || numel(other) == 1)
            error("Objects must be same size for _+_, or one must be scalar.");
        end
    end

    % Quick pass through for "+0" case
    if Util.is_scalar_zero(other)
        val = this;
        return;
    end

    % If other is numeric, convert to monomial:
    if isnumeric(other)
        other = MTKMonomial.InitValue(this.Scenario, other);
    else
        assert(isa(other, 'MTKMonomial'));
        this.checkSameScenario(other);
    end
    
    % If all zeros, then pass through
    %if all(this.Zero)
    

    % Handle remaining cases
    if this.IsScalar && other.IsScalar
        if isequal(this.Operators, other.Operators)
            coef = this.Scenario.Prune(this.Coefficient ...
                                        + other.Coefficient);
            if coef ~= 0
                val = MTKMonomial(this.Scenario, this.Operators, coef);
            else
                val = MTKMonomial(this.Scenario, [], 0);
            end
        else
            val = MTKPolynomial(this.Scenario, [this;other]);
        end
    elseif this.IsScalar % And RHS is not.
        if all(cellfun(@(x) isequal(this.Operators, x), other.Operators))
            [coef, mask] = this.Scenario.Prune(this.Coefficient...
                                             + other.Coefficient);
            if any(mask, 'all')
               new_ops = other.Operators;
               new_ops{mask} = [];
               val = MTKMonomial(this.Scenario, new_ops, coef); 
            else
               val = MTKMonomial(this.Scenario, other.Operators, coef);
            end
        else
            mono_cells = other.split();
            mono_cells = cellfun(@(x) [this; x], mono_cells, ...
                               'UniformOutput', false);
            val = MTKPolynomial(this.Scenario, mono_cells);                    
        end
    elseif other.IsScalar % And LHS is not.
        if all(cellfun(@(x) isequal(x, this.Operators), ...
                this.Operators))
            [coef, mask] = this.Scenario.Prune(this.Coefficient...
                                             + other.Coefficient);
            if any(mask, 'all')
                new_ops = this.Operators;
                new_ops{mask} = [];
                val = MTKMonomial(this.Scenario, new_ops, coef);
            else
                val = MTKMonomial(this.Scenario, this.Operators, coef);
            end
        else 
            mono_cells = this.split();
            mono_cells = cellfun(@(x) [x; other], mono_cells, ...
                               'UniformOutput', false);
            val = MTKPolynomial(this.Scenario, mono_cells);
        end
    else % Nothing is scalar
        if all(cellfun(@(x, y) isequal(x, y), ...
                this.Operators, other.Operators))
            [coef, mask] = this.Scenario.Prune(this.Coefficient ...
                                              + other.Coefficient);
            if any(mask, 'all')
                new_ops = this.Operators;
                new_ops{mask} = [];
                val = MTKMonomial(this.Scenario, new_ops, coef);
            else
                val = MTKMonomial(this.Scenario, this.Operators, coef);
            end
        else                    
            mono_this = this.split();
            mono_other = other.split();
            mono_cells = cellfun(@(x, y) [x; y], ...
                                mono_this, mono_other, ...
                                'UniformOutput', false);
            val = MTKPolynomial(this.Scenario, mono_cells);
        end
    end
end