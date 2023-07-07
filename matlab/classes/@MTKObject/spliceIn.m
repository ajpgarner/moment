function mode = spliceIn(obj, indices, value)
% SPLICEIN Overwrite indexed part of object with supplied values;
% 
% RETURNS Number indicating the type of splice
%   0 - Scalar assignment (scalar -> scalar)
%   1 - Shape-preserving slice assignment
%   2 - Reshaping slice assignment
%   3 - Broadcast (scalar -> many values).
%
% Derived variants should call the base class method first
%     

    % Check scenario matchs
    if value.Scenario ~= obj.Scenario
        error("Can only sub-assign objects belonging to the same scenario.");
    end

    % Probe slice and value dimensions.
    target_dims = cellfun(@numel, indices);
    target_elems = prod(target_dims);
    
    rhs_dims = size(value);
    rhs_elems = numel(value);
      
    % Determine assignment mode.
    if rhs_elems == 1
        if target_elems == 1
            mode = 0;
        else
            mode = 3;
        end
    else
        if rhs_elems ~= target_elems 
            error("Cannot assign: indices yield a slice with %d " + ...
                  "elements, but RHS refers to an object with %d " + ...
                  "elements.", target_elems, rhs_elems);
        end
        
        if isequal(target_dims, rhs_dims)
            mode = 1;
        elseif (numel(rhs_dims) == 1) || (max(rhs_dims) == target_elems)
                mode = 2;
        else
            error("Cannot assign: RHS has a different shape than the indexed slice.");
        end
    end
    
    % Do base splice:
    switch mode
        case 0
            base_splice_scalar(obj, indices, value);
        case 1
            base_splice_shape_preserving(obj, indices, value);
        case 2
            base_splice_reshape(obj, indices, value);
        case 3
            base_splice_broadcast(obj, indices, value);
    end
end

%% Private functions
function obj = base_splice_scalar(obj, indices, value)
    % Can transfer object names:
    if ~isempty(obj.cached_object_name)
        obj.cached_object_name(indices{:}) = value.ObjectName;
    end
   
    % Forget coefficients
    obj.resetCoefficients();
end


function obj = base_splice_shape_preserving(obj, indices, value)
    % Can transfer object names:
    if ~isempty(obj.cached_object_name)
        obj.cached_object_name(indices{:}) = value.ObjectName(:);
    end
    
    % Forget coefficients
    obj.resetCoefficients();
end


function obj = base_splice_reshape(obj, indices, value)
    % Can transfer object names:
    if ~isempty(obj.cached_object_name)
        obj.cached_object_name(indices{:}) = value.ObjectName(:);
    end
    
    % Forget coefficients
    obj.resetCoefficients();
end


function obj = base_splice_broadcast(obj, indices, value)
    % Can transfer object names:
    if ~isempty(obj.cached_object_name)
        obj.cached_object_name(indices{:}) = value.ObjectName;
    end
    
    % Forget coefficients
    obj.resetCoefficients();
end
