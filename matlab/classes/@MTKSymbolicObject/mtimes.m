 function val = mtimes(lhs, rhs)
% TIMES Matrix multiplication *
%
    if (numel(lhs) == 1) || (numel(rhs) == 1)
        val = times(lhs, rhs);
    else
        error("_*_ is only supported when one array is scalar."...
            + " For element-wise multiplication use _.*_");
    end
end