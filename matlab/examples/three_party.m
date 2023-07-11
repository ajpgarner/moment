addpath('..')
clear
clear mtk;

setting = LocalityScenario(3);
setting.Parties(1).AddMeasurement(5);
setting.Parties(1).AddMeasurement(2);
setting.Parties(2).AddMeasurement(3);
setting.Parties(3).AddMeasurement(3);
setting.Parties(3).AddMeasurement(2);

matrix = setting.MomentMatrix(2);

disp(struct2table(matrix.MatrixSystem.SymbolTable));

p_table = matrix.MatrixSystem.ProbabilityTable;
disp(struct2table(p_table))

a15 = setting.Parties(1).Measurements(1).Outcomes(5);
b12 = setting.Parties(2).Measurements(1).Outcomes(2);
c22 = setting.Parties(3).Measurements(2).Outcomes(2);

a15b12 = a15 * b12;
b12c22 = b12 * c22;
a15b12c22_one = a15 * b12c22;
a15b12c22_two = a15b12 * c22;

jm_a2c2 = setting.get([[1, 2]; [3, 2]]);
corr_a2c2 = jm_a2c2.Correlator;