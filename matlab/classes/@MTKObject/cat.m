 function output = cat(join_dimension, varargin)
    % Trivial cases:
    if nargin == 1
        output = MTKObject.empty(0,0);
        return;
    elseif nargin == 2
        output = varargin{1};
        return;
    end

    % Semi-trivial cases (prune empty arrays!)
    mask = ~cellfun(@isempty, varargin);
    if ~any(mask, 'all')
        output = MTKObject.empty(0,0);
        return;
    end
    varargin = varargin(mask);
    if numel(varargin) == 1
        output = varargin(1);
        return;
    end

    % Ensure all inputs are ComplexObjects
    if ~all(cellfun(@(x) isa(x, 'MTKObject'), varargin), 'all')
        error("Can only concatenate MTKObjects");
    end

    % Disable concatenation from different scenarios
    matching_scenario = ...
        all(cellfun(@(x) (varargin{1}.Scenario == x.Scenario),...
                          varargin), 'all');
    if ~matching_scenario                
        error("Can only concatenate MTKObjects from the same scenario.");
    end

    % Disable hetrogenous concatenation
    homogenous = all(cellfun(@(x) strcmp(class(x), class(varargin{1})),...
                             varargin), 'all');
    if ~homogenous
        % TODO: Cast to polynomial, and concatenate
        error("Can only concatenate MTKObjects of the same type.");
    end
    class_name = class(varargin{1});

    % Get output dimensions (fail if inconsistent)
    sizes = cellfun(@size, varargin, 'UniformOutput', false);
    matching_tensor = all(numel(sizes{1}) == cellfun(@(x) numel(x),...
                                                     sizes), 'all');
    if ~matching_tensor
        error("Cannot merge tensors of different dimensionality.");
    end

    % Check matching sizes on dimensions that are not joined 
    nonjoin_dimensions = [1:(join_dimension-1), ...
                          (join_dimension+1):numel(sizes{1})];
    cat_sizes = cellfun(@(x) x(join_dimension), sizes);
    non_cat_sizes = cellfun(@(x) x(nonjoin_dimensions), ...
                            sizes, 'UniformOutput', false);            
    consistent = all(cellfun(@(x) (isequal(non_cat_sizes{1}, x)), non_cat_sizes), 'all');
    if ~consistent
        if numel(nonjoin_dimensions) == 1
            if join_dimension == 1
                error("Cannot vertically concatenate objects with different column sizes.");
            else
                error("Cannot horizontally concatenate objects with different row sizes.");                        
            end
        else
            error("Cannot concatenate objects with mismatched dimensions.");
        end
    end

    % Construct target size
    target_size = zeros(1, numel(sizes{1}));
    target_size(join_dimension) = sum(cat_sizes);
    [target_size(nonjoin_dimensions)] = non_cat_sizes{1};

    % Construct object offsets
    offsets = ones(numel(sizes{1}), numel(varargin));
    offsets(join_dimension, 1:end) = cumsum(cat_sizes);

    % Polymorphic c'tor
    output = feval(class_name + ".InitForOverwrite", ...
                   varargin{1}.Scenario, target_size);

    % Do merge
    output.mergeIn(join_dimension, offsets, varargin);

end