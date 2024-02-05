classdef ValueMatrixTest < MTKTestBase
    %APPLYVALUESTEST Unit tests for apply_values function
    methods (Test, TestTags={'mex'})
        function ImportDense(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            vm_index = mtk('value_matrix', ref_id, ...
                           [1.0, 2.0; 3.0, 4.0], 'label', "Example label");
            
            
            vm_seq = mtk('operator_matrix', 'sequence_string', ...
                         ref_id, vm_index);
            
            vm_reference = ["1", "2"; "3", "4"];            
            testCase.verifyEqual(vm_seq, vm_reference);
            
            vm_name = mtk('operator_matrix', 'name', ref_id, vm_index);
            testCase.verifyEqual(vm_name, "Example label");
            
        end        
        
        function ImportSparse(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            vm_index = mtk('value_matrix', ref_id, ...
                            sparse(eye(5)), 'label', "Identity");
                        
            vm_seq = mtk('operator_matrix', 'sequence_string', ...
                         ref_id, vm_index);
            
            vm_reference = string(eye(5));
            testCase.verifyEqual(vm_seq, vm_reference);
            
            vm_name = mtk('operator_matrix', 'name', ref_id, vm_index);
            testCase.verifyEqual(vm_name, "Identity");            
        end   
    end
end
