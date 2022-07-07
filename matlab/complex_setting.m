clear
clear npatk;

setting = Setting(3);
setting.Parties(1).AddMeasurement(5);
setting.Parties(1).AddMeasurement(2);
setting.Parties(2).AddMeasurement(3);
setting.Parties(3).AddMeasurement(3);
setting.Parties(3).AddMeasurement(2);

matrix = setting.MakeMomentMatrix(2);

disp(struct2table(matrix.SymbolTable));

p_table = npatk("probability_table", matrix);
disp(struct2table(p_table))