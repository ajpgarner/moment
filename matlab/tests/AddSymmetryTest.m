classdef AddSymmetryTest < MTKTestBase
    %ADDSYMMETRYTEST Unit tests for add_symmetry function
     properties(Constant)
        chsh_generators = {[[1 0 0 0 0]; 
                            [1 0 0 0 -1];
                            [0 0 0 1 0];
                            [0 1 0 0 0];
                            [0 0 1 0 0]], ...
                            [[1 0 0 0 0];
                             [0 0 0 0 1];
                             [0 0 0 1 0];
                             [0 0 1 0 0];
                             [0 1 0 0 0]]}
    end

    methods (Test)
        function add_chsh(testCase)
            ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
            [idx,elems] = mtk('add_symmetry', ...
                              ref_id, testCase.chsh_generators);                
            testCase.verifyGreaterThanOrEqual(idx,0);
            testCase.verifyEqual(length(elems), 16);                      
        end
    end
    
    
    methods (Test, TestTags={'Error'})
        function Error_NoInput(testCase)
            function no_in()
                [~, ~] = mtk('add_symmetry');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooManyInputs(testCase)
            function bad_call()                
                ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
                [~,~] = mtk('add_symmetry', ref_id, ...
                            testCase.chsh_generators, ...
                            testCase.chsh_generators);                
            end
            testCase.verifyError(@() bad_call(), 'mtk:too_many_inputs');
        end
    end
end

