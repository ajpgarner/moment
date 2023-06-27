function val = eq(lhs, rhs)
% EQ Compare LHS and RHS for value-wise equality.
%
% SYNTAX
%   1. result = mono_A == mono_B
%   2. result = mono == double
%   3. result = double == mono
%
% RETURNS
%   True if objects are functionally the same, false otherwise.
%
% Syntax 1 will error if objects are not part of the same scenario.
%
% For syntax 1, it is sufficient for equality that the monomials have the 
% same coefficient and operator strings.
%
% For syntaxes 2 and 3, truth requires either the operator string of mono 
% to be empty and the coefficient to match the double; or for the double to
% be zero and the monomial's coefficient to also be zero.
%
% Due to class precedence, "mono == poly" and "poly == mono" are 
% handled by MTKPolynomial.eq.
%
% See also: MTKPOLYNOMIAL.EQ
%

    % Trivially equal if same object
    if eq@handle(lhs, rhs)
        val = true;
        return;
    end

    % Pre-multiplication by a built-in type
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
            error("Objects must be same size for _==_, or one must be scalar.");
        end
    end

    % Compare vs. numeric?
    if isnumeric(other)
        if this.IsScalar
            if numel(other) == 1
                if ~isempty(this.Operators)
                    val = false;
                else
                    val = this.Scenario.IsClose(this.Coefficient, other);
                end                            
            else
                if ~isempty(this.Operators)
                    val = false(size(other));                            
                else
                    val = this.Scenario.IsClose(...
                        repmat(this.Coefficient, size(other)),...
                        other);
                end
            end
        else
            if numel(other) == 1
                val = this.Scenario.IsClose(this.Coefficient,...
                                        repmat(other, size(this)));
                empty = cellfun(@isempty, this.Operators);
                val(empty) = false;
            else
                val = this.Scenario.IsClose(this.Coefficient, other);
                empty = cellfun(@isempty, this.Operators);
                val(empty) = false;                        
            end
        end
        return;
    end

    % Otherwise, compare vs. another monomial
    assert(isa(other, 'MTKMonomial'));

    % Never equal if scenarios do not match
    if this.Scenario ~= other.Scenario
        if this.IsScalar
            val = false(size(other));
        else
            val = false(size(this));
        end
        return
    end

    ops_equal = this.Hash == other.Hash;
    if this.IsScalar
        if other.IsScalar                
            coefs_equal = this.Scenario.IsClose(...
                            this.Coefficient, other.Coefficient);
        else
            coefs_equal = this.Scenario.IsClose(...
                repmat(this.Coefficient, size(other)), ...
                other.Coefficient);
        end
    else
        if other.IsScalar                  
            coefs_equal = this.Scenario.IsClose(this.Coefficient,...
                repmat(other.Coefficient, size(this)));
        else                
            coefs_equal = this.Scenario.IsClose(...
                this.Coefficient, other.Coefficient);
        end
    end
    val = ops_equal & coefs_equal;
end

