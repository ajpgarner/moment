%% First, define locality scenario
scenario = LocalityScenario(2,2,2);
chsh = 0.80;
value_chsh = 2*chsh-1.5;
chsh_ineq = scenario.CGTensor([-value_chsh -1 0; -1 1 1; 0 1 -1]);

%% Optional parameters for the computation
gr_steps = 8;
mm_level = 2;
modeller = 'yalmip';
verbose = true;

%% Now, do BFF calculation
[result, bff_scenario, bff_chsh] = ...
    mtk_bff(scenario, chsh_ineq, 'global', true, ...
            'gr_steps', gr_steps, 'mm_level', mm_level, ...
            'modeller', modeller, 'verbose', verbose);
        
fprintf("Result: %f\n", result);