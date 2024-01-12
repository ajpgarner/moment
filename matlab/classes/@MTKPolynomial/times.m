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

    % Make sure we can multiply
    assert(isa(other, 'MTKObject'), ...
           "__.*__ not defined between %s and %s", class(lhs), class(rhs));

    % Invoke MTK to do multiplication
    [result, is_mono] = mtk('multiply', this.Scenario.System.RefId, ...
                            lhs.OperatorCell, rhs.OperatorCell);    

    if (is_mono) 
        val = MTKMonomial.InitAllInfo(this.Scenario, result{:});
    else
        val = MTKPolynomial.InitFromOperatorPolySpec(this.Scenario, result);
    end
    
 end