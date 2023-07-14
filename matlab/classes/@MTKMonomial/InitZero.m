function obj = InitZero(setting, dimensions)
% INITZERO Initiates an "all zero" monomial array.
      
   % Check inputs
   if nargin < 2 
        error("Must supply setting and values.");
    end    
    if ~isa(setting, 'MTKScenario') 
        error("First argument must be an MTKScenario");
    end
    if ~isnumeric(dimensions)
        error("Second argument must be an array of dimensions.");
    end
    dimensions = double(reshape(dimensions, 1, []));
    
    % Special case: empty.
    if prod(dimensions) == 0
        obj = MTKMonomial.empty(dimensions);
        return;
    end

    % Overwrite
    obj = MTKMonomial(setting, 'overwrite', dimensions);
    obj.symbol_id = zeros(dimensions);
    if prod(dimensions) == 1
        obj.Operators = uint64.empty(1,0);
    else
        cell_dim = num2cell(dimensions);
        obj.Operators = repelem({uint64.empty(1,0)}, cell_dim{:});
    end
    obj.Coefficient = zeros(dimensions);
    obj.Hash = zeros(dimensions);
end