function fc_tensor = mtk_cg2fc(cg_tensor)
%MTK_CG2FC Converts Collins-Gisin tensor to Full Correlator tensor.
% Assumes that every measurement has two outcomes.
%
    % Validate inputs
    assert(nargin>=1 && isnumeric(cg_tensor), ...
        "Argument should be a numeric tensor.");
    
    % Restrict to matrix (for now)
    assert(ismatrix(cg_tensor), ...
        "mtk_fc2gc only supports bipartite systems.");
 
    % Invoke MTK for result
    fc_tensor = mtk('convert_tensor', 'cg2fc', cg_tensor);    
end
