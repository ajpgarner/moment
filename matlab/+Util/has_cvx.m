function test_result = has_cvx()
    %HAS_YALMIP Ascertain whether cvx is installed
    if exist('cvx_version') ~= 2 || exist('cvx_where') ~= 2
        test_result = false;
        return;
    end    
    % TODO
    path = cvx_where();
    test_result = ~isempty(path);
end

