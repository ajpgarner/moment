function varargout = size(obj, varargin)
% SIZE The dimensions of the object.
    
    % Special case for empty objects
    if isempty(obj)
        [varargout{1:nargout}] = builtin('size', obj, varargin{:});
        return;
    end
        
    if nargout <= 1
        if nargin <= 1
            varargout{1} = obj.dimensions;
        else
            if nargin > 2
                error("Too many inputs to function size().");
            end
            if varargin{1} > numel(obj.dimensions)
                varargout{1} = 1; % to match builtin size()
            else 
                varargout{1} = obj.dimensions(varargin{1});
            end
        end
    else
        if nargin ~= 1
            error("Too many outputs provided for size of one dimension.");
        end
        if nargout ~= numel(obj.dimensions)
            error("Object has %d dimensions but %d outputs were provided.", ...
                  numel(obj.dimensions), nargout);
        end
        cell_dim = num2cell(obj.dimensions);
        [varargout{1:nargout}] = cell_dim{:};
    end
end