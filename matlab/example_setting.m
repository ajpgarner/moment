clear
clear npatk;

chsh = Setting(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);

matrix = chsh.MakeMomentMatrix(1);

disp(struct2table(matrix.SymbolTable));
disp(matrix.SymbolMatrix);
disp(matrix.SequenceMatrix);
 
p_table = matrix.ProbabilityTable;
disp(struct2table(p_table))

a22 = chsh.Parties(1).Measurements(2).Outcomes(2);
b22 = chsh.Parties(2).Measurements(2).Outcomes(2);
manual_a22b22 = chsh.get([[1, 2, 2]; [2, 2, 2]]);
prod_a22b22 = a22 * b22;
