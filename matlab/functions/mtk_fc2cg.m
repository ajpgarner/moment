function cg_tensor = mtk_fc2cg(fc_tensor)
%MTK_CG2FC Converts Full Correlator tensor to Collins-Gisin tensor.
% Assumes that every measurement has two outcomes.
%

    % Validate inputs
    assert(nargin>=1 && isnumeric(fc_tensor), ...
        "Argument should be a numeric tensor.");
    
    % Restrict to matrix (for now)
    assert(ismatrix(fc_tensor), ...
        "mtk_fc2gc only supports bipartite systems.");
 
    % Invoke MTK for result
    cg_tensor = mtk('convert_tensor', 'fc2cg', fc_tensor);    
end
