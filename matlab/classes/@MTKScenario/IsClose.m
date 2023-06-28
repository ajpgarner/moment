function output = IsClose(obj, lhs, rhs, scale)
      arguments
         obj (1,1)  MTKScenario
         lhs double,
         rhs double
         scale (1,1) double = 1.0;
      end
     
     % Hack for Mateus' ancient version of MATLAB
     if nargin < 4
         scale = 1.0;
     end
     
     % Check dimensions of arguments
     assert(isequal(size(lhs), size(rhs)) ...
            || numel(lhs) == 1 || numel(rhs) ==1);
        
        
     if ~isreal(lhs) || ~isreal(rhs) % Complex
         diff = (lhs - rhs);
         mod_diff = diff .* conj(diff);
         output = mod_diff < (obj.ZeroTolerance ...
                              * obj.ZeroTolerance ...
                              * eps(scale) * eps(scale));             
     else % Real
         output = abs(lhs - rhs) < obj.ZeroTolerance * eps(scale);
     end
     
 end