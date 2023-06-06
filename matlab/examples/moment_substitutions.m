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
    {{[1, 1], 1}, {[2, 2], 2}, {[3, 3], 3}}, true);
[x, y, z] = setting.getAll();

%% Create default moment matrix
raw_mm = setting.MakeMomentMatrix(1);
fprintf("\nRaw moment matrix #1\n");
disp(raw_mm.SequenceMatrix);

%% Create moment ruleset, with rule <xy> -> i<z>
fprintf("\nRulebook #1\n");
rulebook1 = MomentRuleBook(setting, "Example 1");
rulebook1.Add(x*y - 1i * z);
disp(rulebook1.RuleStrings);

%% Apply ruleset to moment matrix
mm1 = raw_mm.ApplyRules(rulebook1);
disp(mm1.SequenceMatrix);

%% Create moment ruleset, with rules <z> -> <y>, <y> -> <x>
fprintf("\nRulebook #2\n");
rulebook2 = MomentRuleBook(setting, "Example 2");
poly_rules = [z - y; y - x];
rulebook2.Add(poly_rules);
disp(rulebook2.RuleStrings);
mm2 = raw_mm.ApplyRules(rulebook2);
disp(mm2.SequenceMatrix);