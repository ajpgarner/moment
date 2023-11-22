function [output, remainder] = split_varargin_keys(filter, input)
%SPLIT_VARARGIN_KEYS Find keys, and separate them
 
    % Basic validation
    assert(iscell(input) && mod(numel(input), 2)==0, ...
        "Variable inputs should be provided as " ... 
        + "'key1', value1, 'key2', value2...");
   
    output = cell.empty(0, 0);
    remainder = cell.empty(0, 0);
    
    for idx = 1:2:numel(input)
        if ~ischar(input{idx}) && ~(isstring(input{idx}) && (numel(input{idx})==1))
            error("Key in variable input to %s at index %d was not a string.", func_name, idx);
        end
        input{idx} = string(lower(input{idx}));
        
        if ismember(input{idx}, filter)
            output{end+1} = string(lower(input{idx}));
            output{end+1} = input{idx+1};
        else
            remainder{end+1} = string(lower(input{idx}));
            remainder{end+1} = input{idx+1};
        end
    end
end

