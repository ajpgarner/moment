classdef GenerateBasisTest < NPATKTestBase
    %GENERATEBASISTEST Unit tests for generate_basis function
    
    properties(Constant)
        
        % Case 1:
        dense_input = [[1, 2, 3]; [2, 4, -3]; [3, -3, 5]]
        
        string_input = [["1", "2", "3"]; ["2", "4", "-3"]; ["3", "-3", "5"]]
        
        dense_expected_re = {[[1,0,0]; [0,0,0]; [0,0,0]], ... 
                              [[0,1,0]; [1,0,0]; [0,0,0]], ...
                              [[0,0,1]; [0,0,-1]; [1,-1,0]], ...
                              [[0,0,0]; [0,1,0]; [0,0,0]], ...
                              [[0,0,0]; [0,0,0]; [0,0,1]]}
                                                
       sparse_input = sparse([[1, 2, 3]; [2, 4, -3]; [3, -3, 5]])
       
       sparse_expected_re = {sparse([[1,0,0]; [0,0,0]; [0,0,0]]), ... 
                             sparse([[0,1,0]; [1,0,0]; [0,0,0]]), ...
                             sparse([[0,0,1]; [0,0,-1]; [1,-1,0]]), ...
                             sparse([[0,0,0]; [0,1,0]; [0,0,0]]), ...
                             sparse([[0,0,0]; [0,0,0]; [0,0,1]])}
                         
       % Case 2:
       string_input2 = [["1", "2*"]; ["2", "-1"]]       
       string_expected_re2 = {[[1,0]; [0,-1]], [[0,1]; [1,0]]}
       string_expected_im2 = {0, [[0, -1i];[1i, 0]]}       
       string_expected_re_sparse2 = {sparse([[1,0]; [0,-1]]), ...
                                    sparse([[0,1]; [1,0]])}
       string_expected_im_sparse2 = {0, sparse([[0, -1i];[1i, 0]])}
       
       
       % Case 3: Can be made Hermitian, but not symmetric...
       dense_input3 = [[1, 2, 3]; [-2, 4, 5]; [-3, 5, 5]]
       
       string_input3 = [["1","2","3"]; ["-2","4","5"]; ["-3","5","5"]]
       
       dense_expected_re3 = {[[1,0,0]; [0,0,0]; [0,0,0]], ... 
                              0, ...
                              0, ...
                              [[0,0,0]; [0,1,0]; [0,0,0]], ...
                              [[0,0,0]; [0,0,1]; [0,1,1]]}
                          
       dense_expected_im3 = {0, [[0, 1i, 0]; [-1i,0,0]; [0,0,0]], ...
                             [[0, 0, 1i]; [0, 0,0]; [-1i,0,0]], ...
                             0, 0}
                        
       sparse_input3 = sparse([[1, 2, 3]; [-2, 4, 5]; [-3, 5, 5]])
       
       sparse_expected_re3 = {sparse([[1,0,0]; [0,0,0]; [0,0,0]]), ... 
                              0, ...
                              0, ...
                              sparse([[0,0,0]; [0,1,0]; [0,0,0]]), ...
                              sparse([[0,0,0]; [0,0,1]; [0,1,1]])}
                          
       sparse_expected_im3 = {0, ...
                              sparse([[0, 1i, 0]; [-1i,0, 0]; [0,0,0]]), ...
                              sparse([[0, 0, 1i]; [0, 0,0]; [-1i,0,0]]), ...
                              0, 0}

    end
        
    methods
        function verify_sym_output(testCase, output, keys, sparse)
            testCase.assertSize(keys, [5, 2])
            testCase.assertLength(output, 5)
            
            if sparse
                compare_to = testCase.sparse_expected_re;
            else
                compare_to = testCase.dense_expected_re;
            end
            
            % check referred to basis elements match expectations
            for match = 1:5
                index = keys(keys(:,1)==match,2);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output{index}, ...
                    compare_to{match})
            end
        end
        
        function verify_herm_output(testCase, output_re, output_im, ...
                                     keys, sparse)
            testCase.assertSize(keys, [5, 3])
            testCase.assertLength(output_re, 5)
            testCase.assertLength(output_im, 0)
            
            
            if sparse
                compare_to_re = testCase.sparse_expected_re;
            else
                compare_to_re = testCase.dense_expected_re;
            end
            
            % check referred to symmetric basis elements match expectations
            for match = 1:5
                index = keys(keys(:,1)==match,2);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output_re{index}, ...
                    compare_to_re{match})
                
                % But nothing imaginary
                im_index = keys(keys(:,1)==match,3);
                testCase.assertNumElements(im_index , 1)
                testCase.verifyEqual(double(im_index), 0)
            end
         end
        
               
        function verify_herm_output_str2(testCase, output_re, output_im, ...
                                     keys, sparse)
            testCase.assertSize(keys, [2, 3])
            testCase.assertLength(output_re, 2)
            testCase.assertLength(output_im, 1)
                        
            if sparse
                compare_to_re = testCase.string_expected_re_sparse2;
                compare_to_im = testCase.string_expected_im_sparse2;
            else
                compare_to_re = testCase.string_expected_re2;
                compare_to_im = testCase.string_expected_im2;
            end
            
            % check referred to symmetric basis elements match expectations
            for match = 1:2
                index = keys(keys(:,1)==match,2);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output_re{index}, ...
                    compare_to_re{match})
            end
            
            % entries 1 has no imaginary part:
            for match = [1]
                index = keys(keys(:,1)==match,3);
                testCase.assertNumElements(index, 1)
                testCase.verifyEqual(double(index), 0)
            end
            
            % but check for anti-symmetric basis entries of 2
            for match = [2]
                index = keys(keys(:,1)==match,3);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output_im{index}, ...
                    compare_to_im{match})
            end 
         end
                
        function verify_herm_output3(testCase, output_re, output_im, ...
                                     keys, sparse)
            testCase.assertSize(keys, [5, 3])
            testCase.assertLength(output_re, 3)
            testCase.assertLength(output_im, 2)
              
            if sparse
                compare_to_re = testCase.sparse_expected_re3;
                compare_to_im = testCase.sparse_expected_im3;
            else
                compare_to_re = testCase.dense_expected_re3;
                compare_to_im = testCase.dense_expected_im3;
            end
            
            % Real parts
            for match = [1, 4, 5] 
                index = keys(keys(:,1)==match, 2);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output_re{index}, ...
                    compare_to_re{match})
                
                % But nothing imaginary
                im_index = keys(keys(:,1)==match, 3);
                testCase.assertNumElements(im_index , 1)
                testCase.verifyEqual(double(im_index), 0)
            end
            
            % Imaginary parts
            for match = [2, 3]
                index = keys(keys(:,1)==match, 3);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output_im{index}, ...
                    compare_to_im{match})
                
                % But nothing real
                re_index = keys(keys(:,1)==match, 2);
                testCase.assertNumElements(re_index, 1)
                testCase.verifyEqual(double(re_index), 0)
            end 
        end      
        
        function verify_monolith_dense(testCase, cell_re, cell_im, ...
                                       mono_re, mono_im, dim)                                   
            mono_re_dims = size(mono_re);
            testCase.assertEqual(mono_re_dims, ...
                                [length(cell_re), dim*dim]);
                            
            for k = 1:length(cell_re)
                testCase.verifyEqual(reshape(mono_re(k,:), [dim, dim]), ...
                                     cell_re{k});
            end
                        
            % Also test imaginary bits
            if ~isequal(cell_im, 0)
                mono_im_dims = size(mono_im);
                testCase.assertEqual(mono_im_dims, ...
                                    [length(cell_im), dim*dim]);
                for k = 1:length(cell_im)
                    testCase.verifyEqual(reshape(mono_im(k,:), ...
                                                [dim, dim]), ...
                                         cell_im{k});
                end
            elseif ~isequal(mono_im, 0)
                testCase.verifyEqual(isempty(mono_im), true);
            end
        end
        
        function verify_monolith_sparse(testCase, cell_re, cell_im, ...
                                       mono_re, mono_im, dim)
            mono_re_dims = size(mono_re);
            testCase.assertEqual(mono_re_dims, ...
                                [length(cell_re), dim*dim]);
                            
            for k = 1:length(cell_re)
                testCase.verifyEqual(reshape(mono_re(k,:), [dim, dim]), ...
                                     cell_re{k});
            end
                        
            % Also test imaginary bits
            if ~isequal(cell_im, 0)
                mono_im_dims = size(mono_im);
                testCase.assertEqual(mono_im_dims, ...
                                    [length(cell_im), dim*dim]);
                for k = 1:length(cell_im)
                    testCase.verifyEqual(reshape(mono_im(k,:), ...
                                                [dim, dim]), ...
                                         cell_im{k});
                end
            elseif ~isequal(mono_im, 0)
                testCase.verifyEqual(isempty(mono_im), true);
            end
        end
    end
    
    methods (Test)
        function dense_from_dense_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'symmetric', testCase.dense_input);
            testCase.verify_sym_output(output, keys, false);
            
            [mono_output, mono_keys] = ...
                npatk('generate_basis', 'monolith', ...
                      'dense', 'symmetric', testCase.dense_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output, 0, ...
                mono_output, 0, 3);
        end 
        
        function dense_from_dense_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'hermitian', testCase.dense_input);
            testCase.verify_herm_output(output_re, output_im, keys, false);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith', ...
                      'dense', 'hermitian', testCase.dense_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
        
        function dense_from_sparse_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'symmetric', testCase.sparse_input);
            testCase.verify_sym_output(output, keys, false);
            [mono_output, mono_keys] = ...
                npatk('generate_basis', 'monolith', ...
                      'dense', 'symmetric', testCase.sparse_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output, 0, ...
                mono_output, 0, 3);
        end 
        
        function dense_from_sparse_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'hermitian', testCase.sparse_input);
            testCase.verify_herm_output(output_re, output_im, keys, false);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith', ...
                'dense', 'hermitian', testCase.sparse_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
        
        function sparse_from_dense_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'symmetric', testCase.dense_input);
            testCase.verify_sym_output(output, keys, true);
            [mono_output, mono_keys] = ...
                npatk('generate_basis', 'monolith', ...
                      'sparse', 'symmetric', testCase.dense_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output, 0, ...
                mono_output, 0, 3);
        end 
        
        function sparse_from_dense_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'hermitian', testCase.dense_input);
            testCase.verify_herm_output(output_re, output_im, keys, true);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith', ...
                      'sparse', 'hermitian', testCase.dense_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
        
        function sparse_from_sparse_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'symmetric', testCase.sparse_input);
            testCase.verify_sym_output(output, keys, true);
            [mono_output, mono_keys] = npatk('generate_basis', 'monolith', ...
                'sparse', 'symmetric', testCase.sparse_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output, 0, ...
                mono_output, 0, 3);
        end 
        
        function sparse_from_sparse_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'hermitian', testCase.sparse_input);
            testCase.verify_herm_output(output_re, output_im, keys, true);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith', ...
                      'sparse', 'hermitian', testCase.sparse_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
        
        function sparse_from_string_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'symmetric', testCase.string_input);
            testCase.verify_sym_output(output, keys, true);
            [mono_output, mono_keys] = npatk('generate_basis', 'monolith',...
                'sparse', 'symmetric', testCase.string_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output, 0, ...
                mono_output, 0, 3);
        end 
        
        function sparse_from_string_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'hermitian', testCase.string_input);
            testCase.verify_herm_output(output_re, output_im, keys, true);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                'sparse', 'hermitian', testCase.string_input);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
                          
    
        function dense_from_string_hermitian2(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'hermitian', testCase.string_input2);
            testCase.verify_herm_output_str2(output_re, output_im, keys, false);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                'dense', 'hermitian', testCase.string_input2);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output_re, output_im, ...
                mono_output_re, mono_output_im, 2);
        end 
        
        function sparse_from_string_hermitian2(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'hermitian', testCase.string_input2);
            testCase.verify_herm_output_str2(output_re, output_im, keys, true);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                'sparse', 'hermitian', testCase.string_input2);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output_re, output_im, ...
                mono_output_re, mono_output_im, 2);
        end 
      
                     
        function dense_from_dense_hermitian3(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'hermitian', testCase.dense_input3);
            testCase.verify_herm_output3(output_re, output_im, keys, false);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                'dense', 'hermitian', testCase.dense_input3);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
                
        function dense_from_sparse_hermitian3(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'hermitian', testCase.sparse_input3);
            testCase.verify_herm_output3(output_re, output_im, keys, false);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                      'dense', 'hermitian', testCase.sparse_input3);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
        
        function dense_from_string_hermitian3(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'dense', 'hermitian', testCase.string_input3);
            testCase.verify_herm_output3(output_re, output_im, keys, false);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                      'dense', 'hermitian', testCase.string_input3);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_dense(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
        
        function sparse_from_dense_hermitian3(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell',  'sparse', 'hermitian', testCase.dense_input3);
            testCase.verify_herm_output3(output_re, output_im, keys, true);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                'sparse', 'hermitian', testCase.dense_input3);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
        
        function sparse_from_sparse_hermitian3(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'hermitian', testCase.sparse_input3);
            testCase.verify_herm_output3(output_re, output_im, keys, true);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                'sparse', 'hermitian', testCase.sparse_input3);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
      
        function sparse_from_string_hermitian3(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'cell', 'sparse', 'hermitian', testCase.string_input3);
            testCase.verify_herm_output3(output_re, output_im, keys, true);
            [mono_output_re, mono_output_im, mono_keys] = ...
                npatk('generate_basis', 'monolith',...
                'sparse', 'hermitian', testCase.string_input3);
            testCase.verifyEqual(keys, mono_keys);
            testCase.verify_monolith_sparse(output_re, output_im, ...
                mono_output_re, mono_output_im, 3);
        end 
    end
    
    methods (Test, TestTags={'Error'})        
        function Error_NoInput(testCase)
            function no_in()             
               [~, ~] = npatk('generate_basis', 'symmetric');
            end
            testCase.verifyError(@() no_in(), 'npatk:too_few_inputs');           
        end   
        
         function Error_TooManyInputs(testCase)
            function many_in()             
               [~, ~] = npatk('generate_basis', 'symmetric', ...
                              testCase.string_input, ...
                              testCase.string_input, ...
                              testCase.string_input);
            end
            testCase.verifyError(@() many_in(), 'npatk:too_many_inputs');           
        end   
        
        function Error_NoOutput(testCase)
            function no_out()             
               npatk('generate_basis', 'symmetric', testCase.string_input);
            end
            testCase.verifyError(@() no_out(), 'npatk:too_few_outputs');
        end  
        
        function Error_DenseAndSparse(testCase)
            function call_sparse_and_dense()
                [~, ~] = npatk('generate_basis', 'symmetric', ...
                               'sparse', 'dense', ...
                               testCase.string_input);
            end
           testCase.verifyError(@() call_sparse_and_dense, ...
                                'npatk:mutex_param');
        end    
               
        function Error_HermitianAndSymmetric(testCase)
            function call_herm_and_sym()
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'symmetric', 'hermitian', ...
                               testCase.string_input);
            end
           testCase.verifyError(@() call_herm_and_sym, ...
                                'npatk:mutex_param');
        end   
                
        function Error_NonsymmetricInput(testCase)
            function bad_invoke()
                bad_input = [[1, 2]; [3, 3]];
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'symmetric', ...
                               bad_input);
           end
           testCase.verifyError(@() bad_invoke, ...
                                'npatk:bad_param');
        end   
                
        function Error_NonsymmetricInput_TestCase3_Dense(testCase)
            function bad_invoke()
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'symmetric', ...
                               testCase.dense_input3);
           end
           testCase.verifyError(@() bad_invoke, ...
                                'npatk:bad_param');
        end   
        
        function Error_NonsymmetricInput_TestCase3_Sparse(testCase)
            function bad_invoke()
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'symmetric', ...
                               testCase.sparse_input3);
           end
           testCase.verifyError(@() bad_invoke, ...
                                'npatk:bad_param');
        end      
             
        function Error_NonsymmetricInput_TestCase3_String(testCase)
            function bad_invoke()
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'symmetric', ...
                               testCase.string_input3);
           end
           testCase.verifyError(@() bad_invoke, ...
                                'npatk:bad_param');
        end      
        
        function Error_NonsymmetricInput2(testCase)
            function bad_invoke()
                bad_input = [["1", "2"]; ["-2*", "3"]];
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'symmetric', ...
                               bad_input);
           end
           testCase.verifyError(@() bad_invoke, ...
                                'npatk:bad_param');
        end   
        
        function Error_NonhermitianInput(testCase)
            function bad_invoke()
                bad_input = [[1, 2]; [3, 3]];
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'hermitian', ...
                               bad_input);
           end
           testCase.verifyError(@() bad_invoke, ...
                                'npatk:bad_param');
        end   
        
                
        function Error_NonhermitianInput2(testCase)
            function bad_invoke()
                bad_input = [["1", "2"]; ["-2*", "3"]];
                [~, ~] = npatk('generate_basis', 'dense', ...
                               'hermitian', ...
                               bad_input);
           end
           testCase.verifyError(@() bad_invoke, ...
                                'npatk:bad_param');
        end   
    end
end

