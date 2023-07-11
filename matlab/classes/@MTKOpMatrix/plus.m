function val = plus(lhs, rhs)
% PLUS Addition +

    % Special case, sometimes Matrix + Matrix = Matrix
    if isa(lhs,'MTKOpMatrix') && isa(rhs, 'MTKOpMatrix')
        val = combineToLM(lhs, rhs, false);
        if ~islogical(val)
            return;
        end
    end
    
    % Otherwise, downcast and call
    val = degradeAndCall(lhs, rhs, @plus);
    
end