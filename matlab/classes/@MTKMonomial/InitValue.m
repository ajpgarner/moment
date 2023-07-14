function obj = InitValue(setting, values)
% INITVALUE Create an array of monomials representing scalar numeric values
    
    % Check arguments
    if nargin < 2 
        error("Must supply setting and values.");
    end    
    if ~isa(setting, 'MTKScenario') 
        error("First argument must be an MTKScenario");
    end
    if ~isnumeric(values)
        error("Second argument must be a numeric array.");
    end
    
    % Construct
    if isempty(values)
        obj = MTKMonomial.empty(size(values));
    elseif numel(values) == 1
        obj = MTKMonomial(setting, [], double(values));
    else
        dims = num2cell(size(values));
        obj = MTKMonomial(setting, repelem({[]}, dims{:}), double(values));
    end
end