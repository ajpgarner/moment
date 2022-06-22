clear
clear npatk;
chsh = Setting(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);

matrix = chsh.MakeMomentMatrix(1);

disp(matrix.SymbolMatrix);
disp(matrix.SequenceMatrix);
disp(struct2table(matrix.SymbolTable));

