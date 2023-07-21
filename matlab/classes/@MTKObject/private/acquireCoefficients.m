 function acquireCoefficients(obj)
% ACQUIRECOEFFICIENTS Try to find out coefficients.
    try
        [obj.real_coefs, obj.im_coefs] = obj.calculateCoefficients();
               
        obj.has_cached_coefs = true;
        obj.needs_padding = false;
    catch error
        % Set to 0 state
        obj.resetCoefficients();

        % Fail
        rethrow(error);
    end

    % Register symbol update listener 
    if isempty(obj.symbol_added_listener)
        obj.symbol_added_listener = ...
            obj.Scenario.System.listener('NewSymbolsAdded', ...
                                         @obj.onNewSymbolsAdded);
    else
        obj.symbol_added_listener.Enabled = true;
    end
end

