clear
clear npatk;

chsh = Setting(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(1).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);
chsh.Parties(2).AddMeasurement(2);


matrix = chsh.MakeMomentMatrix(1);

cvx_begin sdp quiet
     [a, b, M] = matrix.cvxHermitianBasis();
     a(1) == 1;
     M >= 0;
     
     expression corr_aa
     expression corr_ab
     expression corr_ba
     expression corr_bb
     
     corr_aa = a(1) + 4*a(7) - 2*a(2) - 2*a(4);
     corr_ab = a(1) + 4*a(8) - 2*a(2) - 2*a(5);
     corr_ba = a(1) + 4*a(9) - 2*a(3) - 2*a(4);
     corr_bb = a(1) + 4*a(10) - 2*a(3) - 2*a(5);
     
     expression chsh_ineq;
     chsh_ineq = corr_aa + corr_ab + corr_ba - corr_bb;
        
     maximize(chsh_ineq);
cvx_end

solved_matrix = SolvedMomentMatrix(matrix, a, b);
disp(struct2table(solved_matrix.SymbolTable))

imp_sym = npatk('implied_symbols', matrix)
