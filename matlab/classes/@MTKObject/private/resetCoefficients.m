function resetCoefficients(obj)
% RESETCOEFFICIENTS Forget calculated co-efficients.

	obj.real_coefs = sparse(complex(double.empty(0,0)));
	obj.im_coefs   = sparse(complex(double.empty(0,0)));
    obj.has_cached_coefs = false;
    obj.needs_padding = false;

    if ~isempty(obj.symbol_added_listener)
        obj.symbol_added_listener.Enabled = false;
    end           
end