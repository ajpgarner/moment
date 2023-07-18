%% Example: inflation_CV.m
% Demonstrates algebraic manipulations in continuous variable scenario.
%

%% Define scenario
inflation_level = 2;
moment_matrix_level = 1;

line = InflationScenario(inflation_level, ...
                             [0, 0, 0], ...
                             {[1, 2], [2, 3]});
                         
%% Create plain moment matrix
moment_matrix = line.MomentMatrix(moment_matrix_level);
disp(moment_matrix.SequenceStrings);

%% Extract operators, in form of variant observables.
[A0, A1, B00, B10, B01, B11, C0, C1] = line.getVariants;

%% Create and apply substitution <A0> = 2
rulebook = line.MomentRulebook("Substitutions");
rulebook.Add(A0 - 2);

subbed_matrix = moment_matrix.ApplyRules(rulebook);
disp(subbed_matrix.SequenceStrings);
