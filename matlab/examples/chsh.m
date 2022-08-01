addpath('..')
clear
clear npatk;

setting = Scenario(2);
setting.Parties(1).AddMeasurement(2);
setting.Parties(1).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);

matrix = setting.MakeMomentMatrix(1);

disp(struct2table(matrix.SymbolTable));
disp(matrix.SymbolMatrix);
disp(matrix.SequenceMatrix);
 
p_table = matrix.ProbabilityTable;
disp(struct2table(p_table))

a22 = setting.Parties(1).Measurements(2).Outcomes(2);
b22 = setting.Parties(2).Measurements(2).Outcomes(2);
manual_a22b22 = setting.get([[1, 2, 2]; [2, 2, 2]]);
prod_a22b22 = a22 * b22;

corrA1B1 = Correlator(setting.Parties(1).Measurements(1), ...
                      setting.Parties(2).Measurements(2));
