function [this, other, this_on_lhs] = mapThis(lhs, rhs)
%MAPTHIS Utility function for overloaded operators with dominated classes.
    if ~isa(lhs, 'MTKOpMatrix')
        this = rhs;
        other = lhs;
        this_on_lhs = false;
    else
        this = lhs;
        other = rhs;
        this_on_lhs = true;
    end
end

