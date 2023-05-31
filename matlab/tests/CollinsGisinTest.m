classdef CollinsGisinTest < MTKTestBase
    %COLLINSGISINTEST Unit tests for collins_gisin function
    
    properties(Constant)
    end
    
    methods (Test)
        function CHSH(testCase)
            system_id = mtk('locality_matrix_system', 2, 2, 2);
            mm = mtk('moment_matrix', system_id, 1);
            sym_mat = mtk('collins_gisin', system_id, 'symbols');
            bas_mat = mtk('collins_gisin', system_id, 'basis');
            seq_mat = mtk('collins_gisin', system_id, 'sequences');            
            expected = uint64([[1, 4, 5]; [2, 7, 8]; [3, 9, 10]]);
            testCase.verifyEqual(sym_mat, expected);
            testCase.verifyEqual(bas_mat, expected);
            testCase.verifyEqual(seq_mat, ...
                [["1",   "B.a0", "B.b0"]; ...
                 ["A.a0","A.a0;B.a0","A.a0;B.b0"]; ...
                 ["A.b0","A.b0;B.a0","A.b0;B.b0"]]);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()             
               [~] = mtk('collins_gisin');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');           
        end
        
        
        function Error_MutexParams(testCase)
            function bad_in()
               system_id = mtk('locality_matrix_system', 2, 2, 2);
               [~] = mtk('collins_gisin', system_id, ...
                           'sequences', 'basis');
            end
            testCase.verifyError(@() bad_in(), 'mtk:mutex_param');           
        end  
        
        function Error_TooManyInputs(testCase)
            function bad_in()
               system_id = mtk('locality_matrix_system', 2, 2, 2);
               [~] = mtk('collins_gisin', ...
                              system_id, system_id);
            end
            testCase.verifyError(@() bad_in(), 'mtk:too_many_inputs');           
        end  
        
        function Error_NoMomentMatrix(testCase)
            function bad_in()
               system_id = mtk('locality_matrix_system', 2, 2, 2);
               [~] = mtk('collins_gisin', ...
                              system_id);
            end
            testCase.verifyError(@() bad_in(), 'mtk:missing_cg');           
        end  
        
    end
    
end
