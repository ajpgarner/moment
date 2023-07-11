function val = times(lhs, rhs)
% TIMES Element-wise multiplication .*
    
    % Rescale?
    if (isnumeric(lhs) && numel(lhs) == 1)
        val = rescaleMatrix(rhs, lhs);
        return;
    elseif (isnumeric(rhs) && numel(rhs) == 1)
        val = rescaleMatrix(lhs, rhs);
        return;
    end
    
    % Cast to mono/poly
    val = degradeAndCall(lhs, rhs, @times);
end