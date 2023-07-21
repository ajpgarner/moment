function [val, mask, real_mask, im_mask] = Prune(obj, val, scale)
 % PRUNE Remove terms close to zero.
    
	assert(nargin >= 2 && isnumeric(val));
	
    if nargin < 3
        scale = 1.0;
    end

     % Zero, if close
     mask = abs(val) < (obj.ZeroTolerance * eps(scale));
     val(mask) = 0.0;

     % Real, if close
     real_mask = abs(imag(val)) < (obj.ZeroTolerance * eps(scale));
     val(real_mask) = real(val(real_mask));

     % Imaginary, if close
     im_mask = abs(real(val)) < (obj.ZeroTolerance * eps(scale));
     val(im_mask) = 1i * imag(val(im_mask));             
 end