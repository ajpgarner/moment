clear
clear mtk

[test_dir, ~, ~] = fileparts(mfilename('fullpath'));
test_files = {dir([test_dir, '\*Test.m']).name};

summary = runtests(test_files);
disp(summary);