function val = times(lhs, rhs)
% TIMES Element-wise multiplication .*

    % Pre-multiplication by a built-in type
    if ~isa(lhs, 'MTKMonomial')
        this = rhs;
        other = lhs;
        if ~isnumeric(other)
            error(['Pre-multiplication _*_ should only be ',...
                'invoked when LHS is a built-in numeric type.']);
        end
    else
        this = lhs;
        other = rhs;
    end

    % Check dimensions
    this_size = size(this);
    other_size = size(other);
    if ~isequal(this_size, other_size)
        if ~(this.IsScalar || numel(other) == 1)
            error("Objects must be same size for _.*_, or one must be scalar.");
        end
    end
    
    % If either object is empty, result is empty
    if isempty(this) || isempty(other)
        val = this;
        assert(isempty(val));
        return;
    end
    
    % Handle case special case when other value is numeric
    %  (At some point: this should also be subsumed into mtk `multiply`.)
    if isnumeric(other)
        [new_coefs, prune_mask] = this.Scenario.Prune(this.Coefficient .* other);
        if this.IsScalar
            if numel(other) == 1
                if new_coefs ~= 0
                    % Scaled monomial
                    val = MTKMonomial(this.Scenario, ...
                        this.Operators, new_coefs);
                else
                    % Monomial zero
                    val = MTKMonomial(this.Scenario, [], 0);
                end
            else
                cell_size = num2cell(size(new_coefs));
                new_ops = repelem({this.Operators}, cell_size{:});
                new_ops = MTKUtil.cell_mask(new_ops, prune_mask, []);
                val = MTKMonomial(this.Scenario, ...
                    new_ops, new_coefs);

            end
        else
            new_ops = MTKUtil.cell_mask(this.Operators, prune_mask, []);
            val = MTKMonomial(this.Scenario, new_ops, new_coefs);
        end
        return;
    end
    
    assert(isa(other,'MTKMonomial'));
    assert(this.Scenario.DefinesOperators, ...
        "Monomial multiplication is only defined for scenarios that define operators.");
    
    % Accelerated multiply
    [result, is_mono] = mtk('multiply', this.Scenario.System.RefId, ...
                            lhs.OperatorCell, rhs.OperatorCell);
    assert(is_mono, ...
        "Result of monomial-monomial multiplication was unexpectedly polynomial.");

    val = MTKMonomial.InitAllInfo(this.Scenario, result{:});
end