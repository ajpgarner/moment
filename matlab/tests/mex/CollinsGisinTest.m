classdef CollinsGisinTest < MTKTestBase
    %COLLINSGISINTEST Unit tests for collins_gisin function
    
    methods (Test, TestTags={'mex'})
        function CHSH(testCase)
            system_id = mtk('locality_matrix_system', 2, 2, 2);
            mm = mtk('moment_matrix', system_id, 1);
            [sym_mat, bas_mat] = mtk('collins_gisin', system_id, 'symbols');
            [seq_mat, hash_mat] = mtk('collins_gisin', system_id, 'sequences');            
            expected = uint64([[1, 4, 5]; [2, 7, 8]; [3, 9, 10]]);
            testCase.verifyEqual(sym_mat, expected);
            testCase.verifyEqual(bas_mat, int64(expected-1));
            expected_ops = {uint64.empty(1,0), uint64(3), uint64(4);
                            uint64(1), uint64([1, 3]), uint64([1, 4]);
                            uint64(2), uint64([2, 3]), uint64([2, 4])};
         
            testCase.verifyEqual(seq_mat, expected_ops);
        end
    end
    
    methods (Test, TestTags={'Mex', 'Error'})
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
                           'sequences', 'symbols');
            end
            testCase.verifyError(@() bad_in(), 'mtk:mutex_param');           
        end
        
    end
    
end
