 function [re, im] = calculateCoefficients(obj)
 
    % Early exit if we can't get symbol information for all parts...
    if obj.IsScalar                
        if ~all(obj.Constituents.FoundSymbol, 'all')
            error(obj.err_missing_symbol);
        end
    else
        if ~all(cellfun(@(c) all(c.FoundSymbol, 'all'), ...
                obj.Constituents), 'all')
            error(obj.err_missing_symbol);
        end
    end


    switch obj.DimensionType
        case 0 % SCALAR
            re = obj.Scenario.Prune(...
                    sum(obj.Constituents.RealCoefficients, 2));
            im = obj.Scenario.Prune(...
                    sum(obj.Constituents.ImaginaryCoefficients, 2));

        case {1, 2} % ROW-VECTOR, COL-VECTOR                    
            cell_re = cellfun(@(x) obj.Scenario.Prune(...
                                   sum(x.RealCoefficients, 2)), ...
                              obj.Constituents, ...
                          'UniformOutput', false);
            re = horzcat(cell_re{:});
            cell_im = cellfun(@(x) obj.Scenario.Prune(...
                                   sum(x.ImaginaryCoefficients, 2)), ...
                              obj.Constituents, ...
                          'UniformOutput', false);
            im = horzcat(cell_im{:});

        otherwise                    
            cell_re = cellfun(@(x) obj.Scenario.Prune(...
                                   sum(x.RealCoefficients, 2)), ...
                              obj.Constituents, ...
                          'UniformOutput', false);

            cell_im = cellfun(@(x) obj.Scenario.Prune(...
                                   sum(x.ImaginaryCoefficients, 2)), ...
                              obj.Constituents, ...
                          'UniformOutput', false);

            % Contract index 1 into matrix, distribute over cell
            short_dims = size(cell_re);
            short_dims = short_dims(2:end);
            if (numel(short_dims) == 1)
                re = cell(short_dims, 1);
                im = cell(short_dims, 1);
                for idx=1:prod(short_dims)
                    re{idx} = cell_re{:, idx};
                    im{idx} = cell_im{:, idx};
                end    
            else
                re = cell(short_dims);
                im = cell(short_dims);
                for idx=1:prod(short_dims)
                    re_cell_col = cell_re{:, idx};
                    re{idx} = horzcat(re_cell_col{:});
                    im_cell_col = cell_im{:, idx};
                    im{idx} = horzcat(im_cell_col{:});
                end
            end

    end  
end