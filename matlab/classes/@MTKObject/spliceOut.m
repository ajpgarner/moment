function spliceOut(output, source, indices)
% SPLICEOUT Overwrite output with source data sampled according to indices.
% 
% Derived variants should call the base class method.
% 

    % Do not copy coefficients
    output.has_cached_coefs = false;

    % Copy names
    if ~isempty(source.cached_object_name)
        output.cached_object_name = ...
            source.cached_object_name(indices{:});
    end
end