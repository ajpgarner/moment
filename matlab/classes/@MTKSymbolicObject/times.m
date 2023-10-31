function val = times(lhs, rhs)
    % TIMES Element-wise multiplication .*
    %
    [this, other, ~] = find_this(lhs,rhs);

    % Only support numeric multiplication
    if ~isnumeric(other)
        error(this.err_undefined_op, "_.*_", class(lhs), class(rhs));
    end

    if numel(other) == 1 % Scalar multiplication
        copy_cell = this.SymbolCell;
        for pIdx = 1:numel(copy_cell)
            for mIdx = 1:numel(copy_cell{pIdx})
                copy_cell{pIdx}{mIdx}{2} = ...
                    copy_cell{pIdx}{mIdx}{2} * other;
            end
        end
    elseif numel(this) == 1  % Broadcast over numeric array
        copy_cell = cell(size(other));
        for nIdx = 1:numel(other)
            copy_cell{nIdx} = this.SymbolCell{1};
            for mIdx = 1:numel(copy_cell{nIdx})
                copy_cell{nIdx}{mIdx}{2} = ...
                    copy_cell{nIdx}{mIdx}{2} * other(nIdx);
            end
        end
    else % Element-wise multiplication
        if ~isequal(size(this), size(other))
            error("_.*_ is only supported when array sizes match, or one array is scalar.");
        end
        copy_cell = this.SymbolCell;
        for pIdx = 1:numel(copy_cell)
            for mIdx = 1:numel(copy_cell{pIdx})
                copy_cell{pIdx}{mIdx}{2} = ...
                    copy_cell{pIdx}{mIdx}{2} * other(pIdx);
            end
        end
    end

    % Make new object from rescaled cell
    val = MTKSymbolicObject(this.Scenario, copy_cell);
end