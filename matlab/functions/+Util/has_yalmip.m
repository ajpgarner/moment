function test_result = has_yalmip()
    %HAS_YALMIP Ascertain whether yalmip is installed
    if exist('yalmip') ~= 2
        test_result = false;
        return;
    end
    ymv = str2num(yalmip('version'));
    test_result = ymv > 0;
end

