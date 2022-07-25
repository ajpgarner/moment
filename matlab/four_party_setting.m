clear
clear npatk;

chsh = Setting(4);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);
chsh.Parties(3).AddMeasurement(2);
chsh.Parties(4).AddMeasurement(2);

matrix = chsh.MakeMomentMatrix(2);

disp(struct2table(matrix.SymbolTable));
disp(matrix.SymbolMatrix);
disp(matrix.SequenceMatrix);
 
p_table = matrix.ProbabilityTable;
disp(struct2table(p_table))

a11 = chsh.get([1, 1, 1]);
b11 = chsh.get([2, 1, 1]);
c11 = chsh.get([3, 1, 1]);
d11 = chsh.get([4, 1, 1]);

a11b11 = a11*b11;
c11d11 = c11*d11;
a11c11 = a11*c11;
b11d11 = b11*d11;

a11b11c11d11_one = a11b11 * c11d11;
a11b11c11d11_two = a11c11 * b11d11;
