%% mtk_tests.m
% Runs MATLAB unit tests.

test_dir = get_test_folder();
test_files = find_unit_tests(test_dir);
fprintf("Found %d unit test files...\n", numel(test_files));

old_path = path();
addpath(test_dir);
summary = runtests(test_files);
path(old_path);

disp(summary);

function test_dir = get_test_folder()
    [test_dir, ~, ~] = fileparts(mfilename('fullpath'));
    test_dir = [test_dir, '\tests\'];
end

function files = find_unit_tests(test_dir)    
    test_search_struct = dir([test_dir, '**\*Test.m']);
    names = {test_search_struct.name};
    folders = {test_search_struct.folder};
    files = cellfun(@(a,b) [a, '\', b], folders, names, ...
                    'UniformOutput', false);
end