function varargout = mtk_log(command, varargin)
%MTK_LOG Commands for enabling/disabling and reading the built in logger.

    %  Arguments
    if nargin < 1
        command = "output";
    else
        command = string(command);
    end

    % Invoke logger instruction
    var = false;
    switch(lower(command))
        case "info"
            var = mtk('logging', 'info');
   
        case "output"
            if nargin>=2
                mtk('logging', 'output', varargin{1});
            else
                var = mtk('logging', 'output');
            end
            
        case "file"
            assert(nargin>=2, "You must provide a filename.");
            mtk('logging', 'file', varargin{:});
            
        case "memory"
            mtk('logging', 'memory');
            
        case "off"
            mtk('logging', 'off');
            
        case "clear"
            mtk('logging', 'clear');
            
        otherwise
            error("Unknown log command '%s'.", command);
    end
    
    % Report output
    if nargout == 1
        varargout = {strtrim(var)};
    else
        if ~isequal(var, false)
            fprintf(var);
        end
    end
end

