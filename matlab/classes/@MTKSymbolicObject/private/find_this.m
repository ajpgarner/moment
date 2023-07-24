function [this, other, this_on_lhs] = find_this(lhs, rhs)
    if ~isa(lhs, 'MTKSymbolicObject')
        other = lhs;
        this = rhs;
        this_on_lhs = false;
    else
        this = lhs;
        other = rhs;
        this_on_lhs = true;
    end
end