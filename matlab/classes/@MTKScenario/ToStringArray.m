function str = ToStringArray(obj, input)

    assert(nargin == 2);

    % Do nothing if input is already string
    if isstring(input)
        str = reshape(input, 1, []);
        return;
    end

    % Split up char array
    if ischar(input)
        str = reshape(string(input(:)), 1, []);
        return
    end

    % Complain if not numeric now
    if ~isnumeric(input)
        error("Expected string, character or numeric array");
    end
    
    extended_names = obj.OperatorNames;
    
    % Do nothing if out of range
    if any(input <= 0) || any(input > length(extended_names))
        str = input;
        return;
    end

    % Apply:
    str = reshape(extended_names(input), 1, []);
end