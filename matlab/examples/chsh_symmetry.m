%% Example: chsh_symmetry.m
% Demonstrates Ioannou-Rosset symmetry reduction on the CHSH scenario.

clear
clear mtk

% Settings
mm_level = 1;

% Base scenario
chsh_scenario = LocalityScenario(2, 2, 2);

% Generators of symmetries of the CHSH inequality
chsh_generators = {[[1  1 0 0 0];
                    [0  0 0 1 0];
                    [0  0 0 0 1];
                    [0  0 1 0 0];
                    [0 -1 0 0 0]], ...
                   [[1 0 0 0 0];
                    [0 0 0 0 1];
                    [0 0 0 1 0];
                    [0 0 1 0 0];
                    [0 1 0 0 0]]};
                
% Symmetrized scenario
sym_scenario = SymmetrizedScenario(chsh_scenario, chsh_generators, 2*mm_level);

sym_scenario.System;
mtk('list','verbose');

% Get moment matrices
base_mm = chsh_scenario.MakeMomentMatrix(mm_level);
sym_mm = sym_scenario.MakeMomentMatrix(mm_level);

disp(base_mm.SymbolMatrix);
disp(sym_mm.SymbolMatrix)
