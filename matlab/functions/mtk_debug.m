function varargout = mtk_debug(varargin)
%MTK_DEBUG Wrap mtk in its own process, to allow for debugger attachment.
% NB: Only compatible with MATLAB 2019a and later!
    
    % Create persistent host process
    persistent host_process
    if ~(isa(host_process, 'matlab.mex.MexHost') && isvalid(host_process))
        host_process = mexhost;

        fprintf("Launching MTK debug process with ID: %s.\n", ...
               host_process.ProcessIdentifier);
    end
    
    % Foward inputs/outputs
    varargout = cell(1, nargout);
    [varargout{:}] = feval(host_process, 'mtk', varargin{:});    
end

