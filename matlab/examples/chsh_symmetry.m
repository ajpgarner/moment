clear
clear mtk

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
sym_scenario = SymmetrizedScenario(chsh_scenario, chsh_generators, 2);

sym_scenario.System;
mtk('list','verbose');