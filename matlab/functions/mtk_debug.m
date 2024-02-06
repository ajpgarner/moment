function varargout = mtk_debug(varargin)
%MTK_DEBUG Wrap mtk in its own process, to allow for debugger attachment.
% NB: Only compatible with MATLAB 2019a and later!
%
% For full debug wrapping; consider renaming the mtk compiled executable to 
% _mtk, change 'mtk' to '_mtk' in the feval below, and rename this function 
% file to 'mtk'.
%
% NB: MATLAB host processes can only share data up to 2 GB (presumably due
% to some 32-bit pointers somewhere). Thus, if the above switch is made,
% be warned that things might break for larger moment/localizing matrices.
%
    
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

