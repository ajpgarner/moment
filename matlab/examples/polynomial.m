addpath('..')
clear
clear npatk;

setting = AlgebraicScenario(2, {{[1, 1, 1], []}, ...
                                {[2, 2, 2], []}, ...
                                {[1, 2, 1, 2, 1, 2], []}});
moment_matrix = setting.MakeMomentMatrix(1);