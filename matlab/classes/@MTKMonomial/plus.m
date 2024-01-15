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
    if MTKUtil.is_scalar_zero(other)
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
    
    % Get 'LHS' cell:
    if ~this.Scenario.PermitsSymbolAliases && this.FoundAllSymbols 
        this_cell = this.SymbolCell;
    else
        this_cell = this.OperatorCell;
    end
    
    % Get 'RHS' cell
    if ~this.Scenario.PermitsSymbolAliases && other.FoundAllSymbols
        other_cell = other.SymbolCell;
    else
        other_cell = other.OperatorCell;
    end
    
    [result_cell, is_monomial] = mtk('plus', this.Scenario.System.RefId,...
                                     this_cell, other_cell);

    if is_monomial
        val = MTKMonomial.InitAllInfo(this.Scenario, result_cell{:});
    else
        val = MTKPolynomial.InitFromOperatorPolySpec(...
                                this.Scenario, result_cell);
    end
end
