function test_result = has_yalmip()
%HAS_YALMIP Determine whether YALMIP is available to call from MATLAB.
    if exist('yalmip', 'file') ~= 2
        test_result = false;
        return;
    end
    ymv = str2num(yalmip('version'));
    test_result = ymv > 0;
end

