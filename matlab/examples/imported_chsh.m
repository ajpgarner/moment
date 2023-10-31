%% Example: imported_chsh.m
% Shows how a matrix of symbols can be imported into Moment.
% Expected answer: 2 sqrt 2 (2.828...)

%% Define matrix
mm_str = ["1", "2",  "3",  "4", "5";
          "2", "2",  "6",  "7", "8";
          "3", "6*", "3",  "9", "10";
          "4", "7",  "9",  "4", "11";
          "5", "8",  "10", "11*", "5"];
       
%% Load matrix into scenario
imported_scenario = ImportedScenario();

imported_mm = imported_scenario.ImportHermitianMatrix(mm_str);
disp(imported_scenario.Symbols)

%% Make inequality by manually specifying symbols
chsh_ineq = imported_scenario.ImportPolynomial({{{1, 2.0}, {2, -4.0},...
    {4, -4.0}, {7, 4.0}, {8, 4.0}, {9, 4.0}, {10, -4.0}}});

%% Set-up and solve SDP
mtk_solve(imported_mm, chsh_ineq)
