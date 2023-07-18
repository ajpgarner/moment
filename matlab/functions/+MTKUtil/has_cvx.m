function test_result = has_cvx()
%HAS_YALMIP Determine whether CVX is available to call from MATLAB.
    if exist('cvx_version', 'file') ~= 2 || exist('cvx_where', 'file') ~= 2
        test_result = false;
        return;
    end    
    path = cvx_where();
    test_result = ~isempty(path);
end

