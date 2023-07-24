function merge_type = mergeIn(obj, merge_dim, offsets, objects)
% MERGEIN Joins together complex objects, as part of concatenation.
% Overloads should call this base function first.
% 
    assert(numel(objects)>=2);

    switch (objects{1}.dimension_type)            
        case 0 % Scalar -> 
            if (obj.dimension_type == 1) % Scalar -> Row vector
                merge_type = 0;
            elseif (obj.dimension_type == 2) % Scalar -> Col vector  
                merge_type = 1;
            end

        case 1 % Row Vector -> 
            if (obj.dimension_type == 1)
                merge_type = 2; % Row vector -> Bigger row vector
            else
                merge_type = 4; % Row vector -> Matrix
            end

        case 2 % Col Vector ->
            if (obj.dimension_type == 2)
                merge_type = 3; % Col vector -> Bigger col vector
            else
                merge_type = 5; % Col vector -> Matrix
            end

        case 3
            if (obj.dimension_type == 3)
                merge_type = 6; % Matrix to matrix
            else
                merge_type = 7; % Matrix to tensor
            end
        case 4
            if numel(obj.dimensions) == numel(objects{1}.dimensions)
                merge_type = 8; % Tensor to tensor of same dimension.
            else
                merge_type = 9; % Tensor to tensor of larger dimension.
            end            
        otherwise
            error("Unsupported concatenation type!");
    end


    % Only merge coefficients if all non-empty objects have them:
    hcc = cellfun(@(x) (x.has_cached_coefs), objects);
    if (all(hcc(:)))

        % Do padding on sub-elements, if required
        for idx = 1:numel(objects)
            objects{idx}.padCoefficients();
        end
        
        % Can directly concatenate coefficients, if merging along major
        % direction, or if combining 0/1-dimensional objects.
        if ((merge_type >= 0) && (merge_type <= 3)) ...
            || merge_dim == ndims(obj)
            src_re = cellfun(@(x) x.real_coefs, objects, ...
                             'UniformOutput', false);
            obj.real_coefs = cat(2, src_re{:});
            src_im = cellfun(@(x) x.im_coefs, objects, ...
                             'UniformOutput', false);
            obj.im_coefs = cat(2, src_im{:});
        else % Non-major matrix merge.
            
            % Prepare empty arrays
            obj.real_coefs = zeros(obj.Scenario.System.RealVarCount, ...
                                   numel(obj), 'like', sparse(1i));
            obj.im_coefs = zeros(obj.Scenario.System.ImaginaryVarCount, ...
                                 numel(obj), 'like', sparse(1i));

            % Get strides
            minor_dims = 1:merge_dim;
            major_dims = (merge_dim+1):ndims(obj); % col/last-index-major!
            major_size = size(obj, major_dims);
            num_major = prod(major_size);
            
            rOff = ones(1, numel(objects)); % read-offsets                       
            wIdx = 1; % write offset.
            
            for major = 1:num_major % Iterate over major objects
                for odx = 1:numel(objects) % Iterate over input objects
                    % Copy major object from input object at major index.
                    minor_size = size(objects{odx}, minor_dims);
                    num_minor = prod(minor_size);
                    
                    write_range = wIdx:(wIdx+num_minor-1);
                    read_range = rOff(odx):(rOff(odx)+num_minor-1);
                    
                    obj.real_coefs(:, write_range) = ...
                        objects{odx}.real_coefs(:, read_range);
                        
                    obj.im_coefs(:, write_range) = ...
                        objects{odx}.im_coefs(:, read_range);
                    
                    % Update ranges
                    rOff(odx) = rOff(odx) + num_minor;
                    wIdx = wIdx + num_minor;
                end
            end
        end

        % Register listener for symbol-table updates
        obj.has_cached_coefs = true;
        obj.needs_padding = false;
        obj.symbol_added_listener = ...
            obj.Scenario.System.listener('NewSymbolsAdded', ...
                                         @obj.onNewSymbolsAdded);
    else
        obj.has_cached_coefs = false;
    end
end