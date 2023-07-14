 function obj = InitForOverwrite(setting, dimensions)
% INITFOROVERWRITE Create blank monomial to overwrite.

    % Check inputs
    if nargin < 2 
        error("Must supply setting and dimensions.");
    end    
    if ~isa(setting, 'MTKScenario') 
        error("First argument must be an MTKScenario");
    end
    if ~isnumeric(dimensions)
        error("Second argument must be a dimensions.");
    end
    dimensions = double(reshape(dimensions, 1, []));
     
    % Special case: empty.
    if prod(dimensions) == 0
        obj = MTKMonomial.empty(dimensions);
        return;
    end

    % Otherwise, construct
    obj = MTKMonomial(setting, 'overwrite', dimensions);
end
