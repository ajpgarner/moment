classdef ImportedMatrixSystemTest < MTKTestBase
    %NEWIMPORTEDMATRIXSYSTEMTESTS Unit tests for new_imported_matrix_system
    % mex function
    methods (Test)
        function Empty(testCase)
            ref_id = mtk('imported_matrix_system');
            testCase.verifyGreaterThan(ref_id, 0);
            
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(0));
            testCase.verifyEqual(sys_info.Symbols, uint64(2));
        end
        
        function Real(testCase)
            ref_id = mtk('imported_matrix_system', 'real');
            testCase.verifyGreaterThan(ref_id, 0);
            
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(0));
            testCase.verifyEqual(sys_info.Symbols, uint64(2));
        end
    end
    
    
    methods (Test, TestTags={'Error'})
        function Error_TooManyInputs(testCase)
            function bad_in()
                ref_id = mtk('imported_matrix_system', 1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:too_many_inputs');
        end        
    end
end