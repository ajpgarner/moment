function val = minus(lhs, rhs)
% MINUS Subtraction -

    % Special case, sometimes Matrix - Matrix = Matrix
    if isa(lhs,'MTKOpMatrix') && isa(rhs, 'MTKOpMatrix')
        val = combineToLM(lhs, rhs, true);
        if ~islogical(val) 
            return;
        end
    end
    
    % Otherwise, downcast and call
    val = degradeAndCall(lhs, rhs, @minus);
    
end