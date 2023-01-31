classdef SuggestExtensionsTest < MTKTestBase
    %SUGGESTEXTENSIONSTESTS Unit tests for suggest_extensions mex function
    methods (Test)
        function UnlinkedPair(testCase)            
            ref_id = mtk('new_inflation_matrix_system', [2, 2], {}, 1);
            mm_index = mtk('moment_matrix', ref_id, 1);
            extensions = mtk('suggest_extensions', ref_id, mm_index);
            testCase.verifyEqual(extensions, uint64([2]));
        end
        
        function LinkedPair(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2, 2], {[1 2]}, 1);
            mm_index = mtk('moment_matrix', ref_id, 1);
            extensions = mtk('suggest_extensions', ref_id, mm_index);
            testCase.verifyEqual(extensions, uint64.empty(1,0));
        end       
    end
        
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('suggest_extensions');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()
                sys_id = mtk('new_inflation_matrix_system', [2, 2], {}, 1);
                [~] = mtk('suggest_extensions', sys_id);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
                
        function Error_TooManyInputs(testCase)
            function no_in()                
                sys_id = mtk('new_inflation_matrix_system', [2, 2], {}, 1);
                mm_index = mtk('moment_matrix', sys_id, 1);
                [~] = mtk('suggest_extensions', sys_id, mm_index, mm_index);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_many_inputs');
        end
        
        function Error_WrongSystemType(testCase)
            function no_in()                
                sys_id = mtk('new_locality_matrix_system', 2, 2, 2);
                mm_index = mtk('moment_matrix', sys_id, 1);
                [~] = mtk('suggest_extensions', sys_id, mm_index);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
    end
end