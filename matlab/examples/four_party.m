%% EXAMPLE: four_party.m
% Demonstrates how locality objects can be manipulated.
%

setting = LocalityScenario(4);
setting.Parties(1).AddMeasurement(2);
setting.Parties(1).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);
setting.Parties(3).AddMeasurement(2);
setting.Parties(3).AddMeasurement(2);
setting.Parties(4).AddMeasurement(2);
setting.Parties(4).AddMeasurement(2);

matrix = setting.MomentMatrix(2);

disp(setting.Symbols);
disp(matrix.SymbolStrings);
disp(matrix.SequenceStrings);
 
% Show how to get various operators in locality scenario
a11 = setting.getPMO([1, 1, 1]);
b11 = setting.getPMO([2, 1, 1]);
c11 = setting.getPMO([3, 1, 1]);
d11 = setting.getPMO([4, 1, 1]);

a11b11 = a11*b11;
c11d11 = c11*d11;
a11c11 = a11*c11;
b11d11 = b11*d11;

a11b11c11d11_one = a11b11 * c11d11;
a11b11c11d11_two = a11c11 * b11d11;

small_joint = setting.getPMO([[1 1]; [2 1]]);
big_joint = setting.getPMO([[1 1]; [2 1]; [3 1]; [4 1]]);

as_tensor = big_joint.ImplicitOutcomes;
disp(as_tensor.ObjectName);