function val = plus(lhs, rhs)
    [this, other, ~] = find_this(lhs,rhs);

    % Make other a symbolic object (if possible)
    if isnumeric(other)
        other = MTKSymbolicObject.InitValue(scenario, other);
    elseif isa(other, 'MTKPolynomial') || isa(other, 'MTKMonomial')
        other = MTKSymbolicObject(other);
    elseif ~isa(other, 'MTKSymbolicObject')
        error(this.err_undefined_op, "_+_", class(lhs), class(rhs));
    end

    if numel(this) == 1 % Broadcast this
        symbol_cell = other.SymbolCell;
        for idx=1:numel(symbol_cell)
            symbol_cell{idx} = [symbol_cell{idx}, this.SymbolCell{1}];
        end                
    elseif numel(other) == 1 % Broadcast other
        symbol_cell = this.SymbolCell;
        for idx=1:numel(symbol_cell)
            symbol_cell{idx} = [symbol_cell{idx}, other.SymbolCell{1}];
        end                
    else % Elementwise
        if ~isequal(size(this), size(other))
            error("_+_ is only supported when object sizes match, or one is scalar.");
        end                
        symbol_cell = this.SymbolCell;
        for idx=1:numel(symbol_cell)
            symbol_cell{idx} = [symbol_cell{idx}, other.SymbolCell{idx}];
        end                   
    end

    % Make new object from combined cells
    val = MTKSymbolicObject(this.Scenario, symbol_cell);                        
end