classdef MakeRepresentationTest < MTKTestBase
    %ADDSYMMETRYTEST Unit tests for make_representation function
     properties(Constant)
        chsh_generators = {[[1  1 0 0 0];
                            [0  0 0 1 0];
                            [0  0 0 0 1];
                            [0  0 1 0 0];
                            [0 -1 0 0 0]], ...
                           [[1 0 0 0 0];
                            [0 0 0 0 1];
                            [0 0 0 1 0];
                            [0 0 1 0 0];
                            [0 1 0 0 0]]};
    end

    methods (Test, TestTags={'mex'})
        function add_chsh(testCase)
            base_id = mtk('locality_matrix_system', 2, 2, 2);
            [ref_id,elems] = mtk('symmetrized_matrix_system', ...
                              base_id, testCase.chsh_generators, ...
                              'max_word_length', 2);                
            testCase.verifyGreaterThanOrEqual(ref_id,0);
            testCase.verifyEqual(length(elems), 16);
            
            again_elems = mtk('make_representation', ref_id, 1);
            testCase.verifyEqual(again_elems, elems);
            
            elems_2 = mtk('make_representation', ref_id, 2);
            testCase.assertEqual(length(elems_2), 16);
            for i = 1:length(elems_2)
                mat = elems_2{i};
                testCase.verifyEqual(size(mat), [13 13]);
            end
        end
    end
    
    
    methods (Test, TestTags={'Mex', 'Error'})
        function Error_NoInput(testCase)
            function no_in()
                [~, ~] = mtk('make_representation');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooManyInputs(testCase)
            function bad_call()                
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [idx,~] = mtk('symmetrized_matrix_system', ref_id, ...
                              testCase.chsh_generators, 2);                
                [~] = mtk('make_representation', ref_id, idx, 2, 2);        
            end
            testCase.verifyError(@() bad_call(), 'mtk:too_many_inputs');
        end
    end
end

