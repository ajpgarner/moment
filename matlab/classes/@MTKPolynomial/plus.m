function val = plus(lhs, rhs)
% PLUS Addition.
%
% SYNTAX:
%   1. val = poly + double
%   2. val = double + poly
%   3. val = mono + poly
%   4. val = poly + mono
%   5. val = poly + poly
%
% RETURNS 
%   Either 0, an MTKMonomial, or MTKPolynomial.
%   Numeric 0 is returned if all terms cancel out after addition.
%   MTKMonomial is returned if all but one term cancels out.
%   Otherwise, MTKPolynomial is returned.
%
% See also: ALGEBRAIC.MONOMIAL.PLUS
%

    % Which are we??
    if ~isa(lhs, 'MTKPolynomial')
        this = rhs;
        other = lhs;
    else
        this = lhs;
        other = rhs;
    end

    % Is other side built-in numeric; if so, cast to monomial
    if isnumeric(other)
        other = MTKMonomial.InitValue(this.Scenario, other);
    end

    % Check objects are from same scenario
    this.checkSameScenario(other);
    
    % Get 'LHS' cell:
    if this.Scenario.PermitsSymbolAliases && this.FoundAllSymbols 
        this_cell = this.SymbolCell;
    else
        this_cell = this.OperatorCell;
    end
    
    % Get 'RHS' cell
    if this.Scenario.PermitsSymbolAliases && other.FoundAllSymbols
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