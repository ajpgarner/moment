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
                merge_type = 2; % Col vector -> Bigger col vector
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


    % Only merge coefficients if we have all of them:
    if (all(cellfun(@(x) (x.has_cached_coefs), objects)))

        % Do padding on sub-elements, if required
        for idx = 1:numel(objects)
            objects{idx}.padCoefficients();
        end

        % Combine scalars into row/col-vec,
        % or extend row-vec into bigger row-vec (resp. col-vec)
        if (merge_type >= 0) && (merge_type <= 3)
            obj.real_coefs = objects{1}.real_coefs;
            obj.im_coefs = objects{1}.im_coefs;
            for idx = 2:numel(objects)
                obj.real_coefs = [obj.real_coefs, objects{idx}.real_coefs];
                obj.im_coefs = [obj.im_coefs, ...
                                objects{idx}.im_coefs];
            end
        end

        % Combine rows into matrix:
        if merge_type == 4                    
            % Create empty vectors to recieve values
            num_re = obj.Scenario.System.RealVarCount;
            num_im = obj.Scenario.System.ImaginaryVarCount;
            for col_id = 1:(obj.dimensions(2))
                obj.real_coefs{col_id} = complex(sparse(num_re, 0));
                obj.im_coefs{col_id} = complex(sparse(num_im, 0));
            end

            % Now 'transpose' and deal, because of col-majority...
            for row_id = 1:(obj.dimensions(1))
                for col_id = 1:obj.dimensions(2)
                    obj.real_coefs{col_id} = ...
                        [obj.real_coefs{col_id}, ...
                            objects{row_id}.real_coefs(:, col_id)];
                    obj.im_coefs{col_id} = ...
                        [obj.im_coefs{col_id}, ...
                            objects{row_id}.im_coefs(:, col_id)];
                end
            end                        
        end

        % Combine columns into matrix:
        if merge_type == 5
            % Due to col-majority, straightforward:
            for col_id = 1:(obj.dimensions(2)) % NUMBER OF COLS
                obj.real_coefs{col_id} = objects{col_id}.real_coefs;
                obj.im_coefs{col_id} = objects{col_id}.im_coefs;
            end
        end

        % FIXME: MATRIX AND TENSOR -> ???
        if merge_type >= 6
            error("Tensor merges are not yet supported!");
        end

        % Register listener for symbol-table updates
        obj.has_cached_coefs = true;
        obj.needs_padding = false;
        obj.symbol_added_listener = ...
            obj.Scenario.System.listener('NewSymbolsAdded', ...
                                         @obj.onNewSymbolsAdded);
    end
end