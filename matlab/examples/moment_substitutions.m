%% EXAMPLE: moment_substitutions.m
% Demonstrates how substitutions on the level of moments can be performed,
% to impose equality constraints.
%
clear
clear mtk;

%% Params
mm_level = 1;

%% Create setting with three projectors, get operators
setting = AlgebraicScenario(["x", "y", "z"], ...
    'rules', {{[1, 1], 1}, {[2, 2], 2}, {[3, 3], 3}}, ...
    'hermitian', true, ...
    'tolerance', 10);
[x, y, z] = setting.getAll();

%% Create default moment matrix
raw_mm = setting.MomentMatrix(1);
fprintf("\nRaw moment matrix #1\n");
disp(raw_mm.SequenceStrings);

%% Create moment ruleset, with rule <xy> -> i<z>
fprintf("\nRulebook #1\n");
rulebook1 = setting.MomentRulebook("Example 1");
rulebook1.Add(x*y - 1i * z);
disp(rulebook1.Strings);

%% Apply ruleset to moment matrix
mm1 = raw_mm.ApplyRules(rulebook1);
disp(mm1.SequenceStrings);

%% Create moment ruleset, with rules <z> -> <y>, <y> -> <x>
fprintf("\nRulebook #2\n");
rulebook2 = setting.MomentRulebook("Example 2");
poly_rules = [z - y; y - x];
rulebook2.Add(poly_rules);
disp(rulebook2.Strings);
mm2 = raw_mm.ApplyRules(rulebook2);
disp(mm2.SequenceStrings);