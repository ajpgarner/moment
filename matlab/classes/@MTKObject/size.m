function varargout = size(obj, varargin)
% SIZE Report the dimensions of the object.
%
% Syntax
% 1.   dims = size(obj)
% 2.   [rows, cols, ...] = size(obj)
% 3.   dim = size(obj, dimension)
% 4.   dims = size(obj, [dimension A, dimension B,...])
% 5.   [sizeA, sizeB, ...] = size(obj, [dimension A, dimension B,...])
%
% Syntax 1 and 2 return the entire dimensions of the object, either as a
% row vector (S1) or a structured binding (S2).
% Syntax 3 returns the size along the requested dimension.
% Syntax 4 and 5 return the sizes along the requested dimensions, in order,
% either as a row vector (S4) or as a structured binding (S5).
%
    
    % Special case for empty objects
    if isempty(obj)
        [varargout{1:nargout}] = builtin('size', obj, varargin{:});
        return;
    end
        
    if nargout <= 1
        if nargin <= 1
            varargout{1} = obj.dimensions;
        else
            assert((nargin <= 2) && isnumeric(varargin{1}), ...
              "Second argument to size, if supplied must be dimensions.");
          
            if numel(varargin{1}) > 1
                varargout{1} = zeros(1, numel(varargin{1}));
                for idx = 1:numel(varargin{1})
                    dim = varargin{1}(idx);
                    if dim > numel(obj.dimensions)
                        varargout{1}(idx) = 1;
                    else
                        varargout{1}(idx) = obj.dimensions(dim);
                    end
                end
            else
                if varargin{1} > numel(obj.dimensions)
                    varargout{1} = 1; % to match builtin size()
                else 
                    varargout{1} = obj.dimensions(varargin{1});
                end
            end
        end
    else
        if nargin <= 1
            if nargout < numel(obj.dimensions)
                error("Object has %d dimensions, but only %d outputs were provided.", ...
                      numel(obj.dimensions), nargout);
            end
            cell_dim = num2cell(obj.dimensions);
            [varargout{1:numel(obj.dimensions)}] = cell_dim{:};
            if nargout > numel(obj.dimensions)
                [varargout{(numel(obj.dimensions)+1):nargout}] = deal(1);
            end
        else
            if nargout ~= numel(varargin{1})
                error("Sizes of %d dimensions were requested, but %d outputs were provided.", ...
                    numel(varargin{1}), nargout);
            end
            
             varargout = cell(1, nargout);
             for idx = 1:numel(varargin{1})
                dim = varargin{1}(idx);
                if dim > numel(obj.dimensions)
                    varargout{idx} = 1;
                else
                    varargout{idx} = obj.dimensions(dim);
                end
             end             
        end
    end
end