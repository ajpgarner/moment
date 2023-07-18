function input = check_varargin_keys(allowed_keys, input, func_name)
%CHECK_VARARGIN_KEYS 
    arguments
        allowed_keys (1,:) string
        input (1,:) cell
        func_name (1,1) string = ""
    end
    
    % Reflection to get function name
    if nargin < 3 || isempty(func_name) || isequal(func_name, "")
        stack = dbstack('-completenames');
        if numel(stack) > 1
            func_name = stack(2).name;
        else
            func_name = 'unknown function';
        end
    end
    
    input = reshape(input, 1, []);
    if mod(numel(input), 2) ~= 0
        error("Variable inputs to %s should be provided as 'key1', value1, 'key2', value2...", func_name);
    end
    
    for idx = 1:2:numel(input)
        if ~ischar(input{idx}) && ~(isstring(input{idx}) && (numel(input{idx})==1))
            error("Key in variable input to %s at index %d was not a string.", func_name, idx);
        end
        input{idx} = string(lower(input{idx}));
        
        if ~ismember(input{idx}, allowed_keys)
            error('Key "%s" at index %d is not a recognised parameter for %s.', ...
                  input{idx}, idx, func_name);
        end        
    end
    
end

