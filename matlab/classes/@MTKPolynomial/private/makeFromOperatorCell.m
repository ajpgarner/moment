function makeFromOperatorCell(obj, input)
%MAKEFROMOPERATORCELL Configure according to cell array input.

    % FIXME: Non-scalar input            
    if ~obj.IsScalar
        error("Not yet supported.");
    end

    obj.Constituents = MTKMonomial.empty(1,0);
    for idx = 1:length(input)
        obj.Constituents(end+1) = ...
            MTKMonomial(obj.Scenario, ...
                input{idx}{1}, input{idx}{2});                    
    end
end
