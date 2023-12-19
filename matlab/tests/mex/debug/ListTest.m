classdef ListTest < MTKTestBase
    %LISTTEST Unit tests for list function
    methods (Test, TestTags={'mex'})
        function EmptyList(testCase)            
            empty_list = mtk('list', 'structured');
            testCase.verifyEmpty(empty_list);
        end        
        
         function OneMatrixSystem(testCase)            
            empty_list = mtk('list', 'structured');
            testCase.verifyEmpty(empty_list);
            
            ref_id = mtk('imported_matrix_system');
            list_all = mtk('list', 'structured');
            list_one = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(list_all, list_one);
            testCase.verifyEqual(list_all.RefId, ref_id);
            testCase.verifyEqual(list_all.Matrices, uint64(0));
            testCase.verifyEqual(list_all.Symbols, uint64(2)); % 0 and 1
            testCase.verifyEqual(list_all.Rulebooks, uint64(0));
         end     
               
         function MultiMatrixSystem(testCase)            
            empty_list = mtk('list', 'structured');
            testCase.verifyEmpty(empty_list);
            
            ref_id_A = mtk('imported_matrix_system');
            ref_id_B = mtk('locality_matrix_system', 2, 1, 2);
            mm_index = mtk('moment_matrix', ref_id_B, 1);
            
            list_all = mtk('list', 'structured');
            list_A = mtk('list', 'structured', ref_id_A);
            list_B = mtk('list', 'structured', ref_id_B);
            
            testCase.verifyEqual(list_all(1), list_A);
            testCase.verifyEqual(list_all(2), list_B);
            
            testCase.verifyEqual(list_A.RefId, ref_id_A);
            testCase.verifyEqual(list_A.Matrices, uint64(0));
            testCase.verifyEqual(list_A.Symbols, uint64(2)); % 0 and 1
            testCase.verifyEqual(list_A.Rulebooks, uint64(0));
            
            testCase.verifyEqual(list_B.RefId, ref_id_B);            
            testCase.verifyEqual(list_B.Matrices, uint64(1));
            testCase.verifyEqual(list_B.Symbols, uint64(5)); % 0/1/A/B/AB
            testCase.verifyEqual(list_B.Rulebooks, uint64(0));
        end     
    end
end
