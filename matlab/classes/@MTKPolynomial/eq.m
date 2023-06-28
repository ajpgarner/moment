function val = eq(lhs, rhs)
% EQ Compare LHS and RHS for value-wise equality.
%
% SYNTAX:
%   1. result = poly == double
%   2. result = double == poly
%   3. result = poly == mono
%   4. result = mono == poly
%   5. result = poly == poly
%
% RETURNS
%   True, if objects are functionally the same, false otherwise.
%
% Syntaxes 3, 4 and 5 will error if objects do not share the same
% setting.
%        
% For syntaxes 1 and 2, truth requires either the polynomial have 
% one constituent, and this to be equal to the double (see
% MTKMonomial.EQ), or for the polynomial to be zero as a
% whole, and the double to also be zero.
%
% For syntaxes 3 and 4, truth requires the polynomial contain only 
% one constituent, and that this is equal to the monomial.
%
% For syntax 5, truth requires that the two polynomials contain the
% same number of terms, and these terms are all equivalent.
%

    % Trivially equal if same object
    if eq@handle(lhs, rhs)
        val = true(size(lhs));
        return;
    end

    if isa(lhs, 'MTKPolynomial')
        this = lhs;
        other = rhs;
    else
        this = rhs;
        other = lhs;
    end

    % Handle comparison with numeric
    if isnumeric(other)
        % Make array of scalar values, taking NaN if not scalar.
        this_values = NaN(size(this));
        if this.IsScalar
            if numel(this.Constituents) == 0
                this_values = 0;
            elseif numel(this.Constituents) == 1 && ...
                    isempty(this.Constituents(1).Operators)
                this_values = this.Constituents.Coefficient;
            end
        else                    
            for idx=1:numel(this.Constituents)
                if numel(this.Constituents{idx}) == 0
                    this_values(idx)= 0;
                elseif numel(this.Constituents{idx}) == 1 && ...
                    isempty(this.Constituents{idx}.Operators)
                    this_values(idx) = this.Constituents{idx}.Coefficient;
                end
            end
        end
        % Compare vs. other
        val = this_values == other;
        return;            
    end

    % Never equal if scenarios do not match
    assert(isa(other, 'MTKObject'));
    if this.Scenario ~= other.Scenario
        if this.IsScalar
            val = false(size(other));
        else
            val = false(size(this));
        end
        return
    end

    % Handle comparison with monomials
    if isa(other, 'MTKMonomial')
        if this.IsScalar
            if other.IsScalar
                val = ((numel(this.Constituents) == 1)  ...
                       && this.Constituents == other) ...
                    || ((numel(this.Constituents) == 0) ...
                        && other.IsZero);
            else
                if this.IsZero
                    val = other.IsZero;
                elseif numel(this.Constituents) == 1
                    val = this.Constituents(1) == other;
                else
                    val = false(size(other));
                end
            end
        else
            if other.IsScalar
                if other.IsZero
                    val = this.IsZero;
                else
                    val = cellfun(@(x) (numel(x)==1 && x == other),...
                        this.Constituents);
                end
            else
                % First of all, match places where this and other are zero.
                val = this.IsZero & other.IsZero;
                where_one = cellfun(@(x) (numel(x)==1),...
                    this.Constituents);
                where_one = where_one & ~other.IsZero;

                for idx=1:numel(this)
                    if where_one(idx)
                        val(idx) = ...
                            isequal(this.Constituents{idx}.Hash, other.Hash(idx)) ...
                          && this.Scenario.IsClose(this.Constituents{idx}.Coefficient, ...
                                                  other.Coefficient(idx));

                    end
                end
            end
        end
        return;
    end
    
    % Handle polynomial case
    assert (isa(other, 'MTKPolynomial'))
    if this.IsScalar
        if other.IsScalar
            val = isequal(this.Constituents, other.Constituents);
        else
            val = cellfun(@(x) isequal(this.Consistutents, x), ...
                          other.Constituents);
        end
    else
        if other.IsScalar
            val = cellfun(@(x) isequal(x, other.Consistutents), ...
                          this.Constituents);
        else
            val = cellfun(@isequal, this.Constituents, ...
                          other.Constituents);
        end
    end
end
