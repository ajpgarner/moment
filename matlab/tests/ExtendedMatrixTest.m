classdef ExtendedMatrixTest < MTKTestBase
    %APPLYVALUESTEST Unit tests for apply_values function
    methods (Test)
        function ExtendManual(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2, 2], {}, 1);
            mm_index = mtk('moment_matrix', ref_id, 1);
            em_index = mtk('extended_matrix', ref_id, 1, [2]);
            
            mm_seq = mtk('operator_matrix', 'sequences', ref_id, mm_index);
            em_seq = mtk('operator_matrix', 'sequences', ref_id, em_index);
            em_reference = strings(4,4);
            em_reference(1:3,1:3) = mm_seq;
            em_reference(1,4) = "<A>";
            em_reference(2,4) = "<A><A>";
            em_reference(3,4) = "<A><B>";
            em_reference(4,1) = "<A>";
            em_reference(4,2) = "<A><A>";
            em_reference(4,3) = "<A><B>";
            em_reference(4,4) = "<A><A>";
            testCase.verifyEqual(em_seq, em_reference);
            
            mm_sym = mtk('operator_matrix', 'symbols', ref_id, mm_index);
            em_sym = mtk('operator_matrix', 'symbols', ref_id, em_index);
            em_sym_ref = strings(4,4);
            em_sym_ref(1:3, 1:3) = mm_sym;
            em_sym_ref(1,4) = "2";
            em_sym_ref(2,4) = "5";
            em_sym_ref(3,4) = "4";
            em_sym_ref(4,1) = "2";
            em_sym_ref(4,2) = "5";
            em_sym_ref(4,3) = "4";
            em_sym_ref(4,4) = "5";
            testCase.verifyEqual(em_sym, em_sym_ref);
        end        
        
        function ExtendTwice(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2, 2], {}, 1);
            mm_index = mtk('moment_matrix', ref_id, 1);
            em_index = mtk('extended_matrix', ref_id, 1, [2, 3]);
            
            mm_seq = mtk('operator_matrix', 'sequences', ref_id, mm_index);
            em_seq = mtk('operator_matrix', 'sequences', ref_id, em_index);
            em_reference = [["1", "<A>", "<B>", "<A>", "<B>"]; ...
                            ["<A>", "<A>", "<A><B>", "<A><A>", "<A><B>"];...
                            ["<B>", "<A><B>", "<B>", "<A><B>", "<B><B>"];...
                            ["<A>", "<A><A>", "<A><B>", "<A><A>", "<A><B>"];...
                            ["<B>", "<A><B>", "<B><B>", "<A><B>", "<B><B>"]];
            testCase.verifyEqual(mm_seq, em_reference(1:3,1:3));
            testCase.verifyEqual(em_seq, em_reference);
            
            mm_sym = mtk('operator_matrix', 'symbols', ref_id, mm_index);
            em_sym = mtk('operator_matrix', 'symbols', ref_id, em_index);
            em_sym_ref = [["1", "2", "3", "2", "3"];...
                          ["2", "2", "4", "5", "4"];...
                          ["3", "4", "3", "4", "6"];...
                          ["2", "5", "4", "5", "4"];...
                          ["3", "4", "6", "4", "6"]];
            testCase.verifyEqual(mm_sym, em_sym_ref(1:3, 1:3));
            testCase.verifyEqual(em_sym, em_sym_ref);
        end     
        
        function ExtendAutomatic(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2, 2], {}, 1);
            mm_index = mtk('moment_matrix', ref_id, 1);
            em_index = mtk('extended_matrix', ref_id, 1, 'auto');
            
            mm_seq = mtk('operator_matrix', 'sequences', ref_id, mm_index);
            em_seq = mtk('operator_matrix', 'sequences', ref_id, em_index);
            em_reference = strings(4,4);
            em_reference(1:3,1:3) = mm_seq;
            em_reference(1,4) = "<A>";
            em_reference(2,4) = "<A><A>";
            em_reference(3,4) = "<A><B>";
            em_reference(4,1) = "<A>";
            em_reference(4,2) = "<A><A>";
            em_reference(4,3) = "<A><B>";
            em_reference(4,4) = "<A><A>";
            testCase.verifyEqual(em_seq, em_reference);
            
            mm_sym = mtk('operator_matrix', 'symbols', ref_id, mm_index);
            em_sym = mtk('operator_matrix', 'symbols', ref_id, em_index);
            em_sym_ref = strings(4,4);
            em_sym_ref(1:3, 1:3) = mm_sym;
            em_sym_ref(1,4) = "2";
            em_sym_ref(2,4) = "5";
            em_sym_ref(3,4) = "4";
            em_sym_ref(4,1) = "2";
            em_sym_ref(4,2) = "5";
            em_sym_ref(4,3) = "4";
            em_sym_ref(4,4) = "5";
            testCase.verifyEqual(em_sym, em_sym_ref);
        end     
    end
end
