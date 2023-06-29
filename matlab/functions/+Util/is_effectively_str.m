function result = is_effectively_str(input)
%IS_EFFECTIVELY_STR Is this a single string for all intents and purposes?
%
% Examples of true inputs:
%   "hello world"
%   'property'
%
% Examples of false inputs:
%   42
%   string.empty(1,0)
%   ["hello", "world"]
%
    result = isStringScalar(input) || ischar(input);
end

