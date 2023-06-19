clear mtk

[test_dir, ~, ~] = fileparts(mfilename('fullpath'));
test_dir = [test_dir, '\tests\'];
test_files = {dir([test_dir, '*Test.m']).name};
test_files = cellfun(@(str)[test_dir, str], test_files, 'uni', false);

summary = runtests(test_files);
disp(summary);