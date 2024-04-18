[moment_dir, ~, ~] = fileparts(mfilename('fullpath'));
addpath(moment_dir, [moment_dir, '/classes'], [moment_dir, '/functions']);
result = savepath;
assert(result == 0, "A problem occured while attempting to save the path.");
