%% Example: complete.m
% Demonstrates Knuth-Bendix reduction in the algebraic scenario.
% Expected result: ultimately completes.

scenario = AlgebraicScenario('xy', 'hermitian', false, 'normal', false);
rulebook = scenario.OperatorRulebook;
rulebook.AddRule('xxx', []);
rulebook.AddRule('yyy', []);
rulebook.AddRule('xyxyxy', []);
rulebook.Complete(100, true);
