%% Clear
addpath('..')
clear
clear mtk;

%% Gen group
a = [[1 0 0 0 0]; 
     [1 0 0 0 -1];
     [0 0 0 1 0];
     [0 1 0 0 0];
     [0 0 1 0 0]];
 
r = [[1 0 0 0 0];
     [0 0 0 0 1];
     [0 0 0 1 0];
     [0 0 1 0 0];
     [0 1 0 0 0]];

generators = {a, r};

chsh_group = Symmetry.Group(generators)
chsh_group.Generate();
chsh_group