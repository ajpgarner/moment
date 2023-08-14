%% chsh_terse.m
% The "hello world" of quantum SDPs. 
% Creates a Bell scenario, and optimizes to find minimal value of CHSH.
% Expected result: -2.8284

scenario = LocalityScenario(2, 2, 2);
chsh = scenario.FCTensor([0 0 0; 0 1 1; 0 1 -1]);
result = mtk_solve(scenario.MomentMatrix(1), chsh)
