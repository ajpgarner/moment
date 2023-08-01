function val = mtk_multithreading(new_val)
% MTK_LOCALITY_FORMAT Gets/set global "multithreading policy" setting.
% Current options are 'off', 'auto', 'always'. Setting 'on' is provided as 
% an alias for 'auto'.
%
% It is not recommended to use 'always', as the costs associated with
% launching new threads may (greatly) exceed the time saved from
% parallelization for easier tasks.
%

    % If no arguments, get MT policy
    if nargin == 0        
        s = mtk('settings', 'structured');
        val = s.multithreading;
        return;
    end
    
    % Validate argument
    assert(nargin == 1 && MTKUtil.is_effectively_str(new_val), ...
           "Argument, if provided, must be string 'off', 'on' or 'always'");
    new_val = lower(char(new_val));
    
    % Change MT policy
    s = mtk('settings', 'structured', 'multithreading', new_val);
    val = s.multithreading;
end
