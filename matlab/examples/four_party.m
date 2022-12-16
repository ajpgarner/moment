addpath('..')
clear
clear mtk;

setting = LocalityScenario(4);
setting.Parties(1).AddMeasurement(2);
setting.Parties(1).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);
setting.Parties(3).AddMeasurement(2);
setting.Parties(3).AddMeasurement(2);
setting.Parties(4).AddMeasurement(2);
setting.Parties(4).AddMeasurement(2);

matrix = setting.MakeMomentMatrix(2);

disp(struct2table(matrix.MatrixSystem.SymbolTable));
disp(matrix.SymbolMatrix);
disp(matrix.SequenceMatrix);
 
p_table = matrix.MatrixSystem.ProbabilityTable;
disp(struct2table(p_table))

a11 = setting.get([1, 1, 1]);
b11 = setting.get([2, 1, 1]);
c11 = setting.get([3, 1, 1]);
d11 = setting.get([4, 1, 1]);

a11b11 = a11*b11;
c11d11 = c11*d11;
a11c11 = a11*c11;
b11d11 = b11*d11;

a11b11c11d11_one = a11b11 * c11d11;
a11b11c11d11_two = a11c11 * b11d11;
