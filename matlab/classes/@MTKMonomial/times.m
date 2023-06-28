function val = times(lhs, rhs)
% TIMES Element-wise multiplication .*

    % Pre-multiplication by a built-in type
    if ~isa(lhs, 'MTKMonomial')
        this = rhs;
        other = lhs;
        if ~isnumeric(other)
            error(['Pre-multiplication _*_ should only be ',...
                'invoked when LHS is a built-in type.']);
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

    % Handle case when other value is numeric
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
                new_ops = Util.cell_mask(new_ops, prune_mask, []);
                val = MTKMonomial(this.Scenario, ...
                    new_ops, new_coefs);

            end
        else
            new_ops = Util.cell_mask(this.Operators, prune_mask, []);
            val = MTKMonomial(this.Scenario, new_ops, new_coefs);
        end
        return;
    end

    assert(isa(other,'MTKMonomial'));

    % Handle various cases of Monomial with Monomial:
    [join_coefs, prune_mask] = ...
        lhs.Scenario.Prune(lhs.Coefficient .* rhs.Coefficient);

    if lhs.IsScalar && rhs.IsScalar
        if join_coefs ~= 0
            new_ops = [lhs.Operators, rhs.Operators];
        else
            new_ops = [];
        end
    elseif lhs.IsScalar
        new_ops = cellfun(@(x) [lhs.Operators, x], rhs.Operators, ...
            'UniformOutput', false);
        new_ops = Util.cell_mask(new_ops, prune_mask, []);
    elseif rhs.IsScalar
        new_ops = cellfun(@(x) [x, rhs.Operators], lhs.Operators, ...
            'UniformOutput', false);
        new_ops = Util.cell_mask(new_ops, prune_mask, []);
    else
        new_ops = cellfun(@(x, y) [x, y], ...
            lhs.Operators, rhs.Operators, 'UniformOutput', false);
        new_ops = Util.cell_mask(new_ops, prune_mask, []);
    end
    val = MTKMonomial(this.Scenario, new_ops, join_coefs);
end