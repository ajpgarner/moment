function spliceOut(output, source, indices)
% SPLICEOUT Overwrite output with source data sampled according to indices.
% 
% Derived variants should call the base class method.
% 

    % Copy coefficients if known
    if source.has_cached_coefs
        % Get flattened indices
        flat_indices = MTKUtil.sub_to_index(size(source), indices);
        
        % Trigger pad if necessary
        src_re = source.RealCoefficients;
        src_im = source.ImaginaryCoefficients;        
        output.real_coefs = src_re(:, flat_indices);
        output.im_coefs = src_im(:, flat_indices);           
        output.has_cached_coefs = true;
        output.needs_padding = false;

        % Register listener
        assert(isempty(output.symbol_added_listener));
        output.symbol_added_listener = ...
            output.Scenario.System.listener('NewSymbolsAdded', ...
                                            @output.onNewSymbolsAdded);   
    else
        output.has_cached_coefs = false;
    end

    % Copy names if known
    if ~isempty(source.cached_object_name)
        output.cached_object_name = ...
            source.cached_object_name(indices{:});
    end
end