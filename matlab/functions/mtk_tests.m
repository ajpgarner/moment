function summary = mtk_tests(tags)
% MTK_TESTS Find and run Moment unit tests.
%
% SYNTAX
%    1. result = mtk_tests();
%    2. result = mtk_tests('tag');
%    3. result = mtk_tests({'tagA', 'tagB'});
%
% RETURNS
%   Test result summary object.
%

    % Find files
    test_dir = get_test_folder();
    test_files = find_unit_tests(test_dir);
    fprintf("Found %d unit test files.\n__________\n\n", numel(test_files));

    % Pass forward tags as argument
    extra_args = cell(1,0);
    if nargin >= 1
        if ~isempty(tags)
            if numel(tags) == 1
                extra_args = [extra_args, 'Tag', tags];
            else
                extra_args = [extra_args, 'Tag', {tags}];
            end        
        end
    end

    % Run tests
    old_path = path();
    addpath(test_dir);    
    summary = runtests(test_files, extra_args{:});
    path(old_path);

    % Display output, if non supplied
    if nargout < 1
        disp(summary);
    end
end

%% Private functions
function test_dir = get_test_folder()
    [test_dir, ~, ~] = fileparts(mfilename('fullpath'));
    slashes = strfind(test_dir, filesep);
    test_dir = test_dir(1:slashes(end)-1);
    test_dir = [test_dir, filesep, 'tests', filesep];
end

function files = find_unit_tests(test_dir)    
    test_search_struct = dir([test_dir, '**', filesep, '*Test.m']);
    names = {test_search_struct.name};
    folders = {test_search_struct.folder};
    files = cellfun(@(a,b) [a, filesep, b], folders, names, ...
                    'UniformOutput', false);
end