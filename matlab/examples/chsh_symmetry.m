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
sym_scenario = SymmetrizedScenario(chsh_scenario, chsh_generators);

% Get representations and averages
rep1 = sym_scenario.Group.Representation(1);
rep2 = sym_scenario.Group.Representation(2);

format short
disp(full(rep2.Average));

%sym_mm = sym_scenario.MakeMomentMatrix(1);

%r2a = rep2.Average;

%[u,s,v] = svds(r2a,2)