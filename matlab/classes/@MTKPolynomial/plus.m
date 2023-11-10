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
    
    % Do we have symbolic representation for accelerated addition?
    if this.FoundAllSymbols && other.FoundAllSymbols
        result_cell = mtk('plus', this.Scenario.System.RefId, ...
                          this.SymbolCell, other.SymbolCell);
        celldisp(result_cell);
        val = MTKPolynomial.InitFromOperatorPolySpec(this.Scenario, ...
                                                     result_cell);
        return;
    end

    % Handle Monomial append case
    if isa(other, 'MTKMonomial')
        if this.IsScalar
            if other.IsScalar
                val = MTKPolynomial(this.Scenario, ...
                                [this.Constituents; other]);
            else
                other = other.split();                        
                val = MTKPolynomial(this.Scenario, ...
                    cellfun(@(x) [this.Constituents; x], other,...
                            'UniformOutput', false));
            end
        else
            if other.IsScalar
               val = MTKPolynomial(this.Scenario, ...
                    cellfun(@(x) [x; other], ...
                        this.Constituents, 'UniformOutput', false));
            else
                other = other.split();
                val = MTKPolynomial(this.Scenario, ...
                    cellfun(@(x, y) [x; y], ...
                        this.Constituents, other, ...
                        'UniformOutput', false));
            end                    
        end                
        return
    end

    % Handle Polynomial append case
    assert(isa(other, 'MTKPolynomial'));
    if this.IsScalar
        if other.IsScalar
            val = MTKPolynomial(this.Scenario, ...
                            [this.Constituents; ...
                             other.Constituents]);
        else
            val = MTKPolynomial(this.Scenario, ...
                cellfun(@(x) [this.Constituents; x], ...
                        other.Constituents, ...
                        'UniformOutput', false));
        end
    else
        if other.IsScalar
           val = MTKPolynomial(this.Scenario, ...
                cellfun(@(x) [x; other.Constituents], ...
                    this.Constituents, 'UniformOutput', false));
        else
            val = MTKPolynomial(this.Scenario, ...
                cellfun(@(x, y) [x; y], ...
                    this.Constituents, other.Constituents, ...
                    'UniformOutput', false));
        end
    end            
end