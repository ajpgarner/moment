addpath('..')
clear
clear mtk;

setting = LocalityScenario(2);
setting.Parties(1).AddMeasurement(2);
setting.Parties(1).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);
setting.Parties(2).AddMeasurement(2);

matrix = setting.MomentMatrix(10);

disp(matrix.SymbolStrings);
disp(matrix.SequenceStrings);
disp(setting.Symbols());

a22 = setting.Parties(1).Measurements(2).Outcomes(2);
b22 = setting.Parties(2).Measurements(2).Outcomes(2);
manual_a22b22 = setting.get([[1, 2, 2]; [2, 2, 2]]);
prod_a22b22 = a22 * b22;

corrA1B1 = setting.Parties(1).Measurements(1).Correlator(...
                      setting.Parties(2).Measurements(1));

chsh_ineq = setting.FCTensor([[0 0 0]; [0 1 1]; [0 1 -1]]);
chsh_ineq2 = setting.CGTensor([[2 -4 0]; [-4 4 4]; [0 4 -4]]);
