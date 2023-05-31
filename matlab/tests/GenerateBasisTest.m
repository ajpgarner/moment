classdef GenerateBasisTest < MTKTestBase
    %GENERATEBASISTEST Unit tests for generate_basis function
    
    properties(Constant)
        % Case 1 (Real):
        dense_input1 = [[1, 2, 3]; [2, 4, -3]; [3, -3, 5]]
        
        dense_expected_re1 = {[[1,0,0]; [0,0,0]; [0,0,0]], ...
            [[0,1,0]; [1,0,0]; [0,0,0]], ...
            [[0,0,1]; [0,0,-1]; [1,-1,0]], ...
            [[0,0,0]; [0,1,0]; [0,0,0]], ...
            [[0,0,0]; [0,0,0]; [0,0,1]]}
        
        sparse_expected_re1 = {sparse([[1,0,0]; [0,0,0]; [0,0,0]]), ...
            sparse([[0,1,0]; [1,0,0]; [0,0,0]]), ...
            sparse([[0,0,1]; [0,0,-1]; [1,-1,0]]), ...
            sparse([[0,0,0]; [0,1,0]; [0,0,0]]), ...
            sparse([[0,0,0]; [0,0,0]; [0,0,1]])}
        
        % Case 2 (Complex):
        string_input2 = [["1", "2*"]; ["2", "-1"]]
        
        dense_expected_re2 = {[[1,0]; [0,-1]], [[0,1]; [1,0]]}
        dense_expected_im2 = {[[0, -1i];[1i, 0]]}
        sparse_expected_re2 = {sparse([[1,0]; [0,-1]]), ...
            sparse([[0,1]; [1,0]])}
        sparse_expected_im2 = {sparse([[0, -1i];[1i, 0]])}
        
        
    end
    
    methods(Access=protected)
        function verify_cell(testCase, basis_re, basis_im, keys, ...
                ref_re, ref_im)
            
            % Check real
            expt_re_length = length(ref_re);
            if expt_re_length > 0
                testCase.assertEqual(length(basis_re), expt_re_length);
                for match = 1:expt_re_length
                    
                    % Symbol number -> real element?
                    index = keys(keys(:,1)==match,1);
                    
                    testCase.assertNumElements(index, 1)
                    
                    testCase.assertGreaterThan(index, 0)
                    testCase.verifyEqual(basis_re{index}, ...
                        ref_re{match})
                end
            else
                testCase.verifyTrue(isempty(basis_re));
            end
            
            % Check imaginary
            expt_im_length = length(ref_im);
            if expt_im_length > 0
                testCase.assertEqual(length(basis_im), expt_im_length);
                for match = 1:expt_im_length
                    index = keys(keys(:,1)==match,2);
                    testCase.assertNumElements(index, 1)
                    testCase.assertGreaterThan(index, 0)
                    testCase.verifyEqual(basis_im{index}, ...
                        ref_im{match})
                end
            else
                testCase.verifyTrue(isempty(basis_im))
            end
        end
        
        function verify_monolith(testCase, ...
                basis_re, basis_im, keys, ...
                ref_re, ref_im)
            % Get basis dimensions
            expanded_dims = size(ref_re{1});
            dim = expanded_dims(1);
            
            % Check dimensions of real
            expt_re_length = length(ref_re);
            expt_re_dims = [dim*dim, expt_re_length];
            testCase.assertEqual(size(basis_re), expt_re_dims);
            
            % Rebuild real cell array
            rebuilt_cell_re = cell(1, expt_re_length);
            for k = 1:length(ref_re)
                rebuilt_cell_re{k} = reshape(basis_re(:, k), [dim, dim]);
            end
            
            % Check dimensions of imaginary
            expt_im_length = length(ref_im);
            mono_im_dims = size(basis_im);
            testCase.assertEqual(mono_im_dims, [dim*dim, expt_im_length]);
            
            % Rebuild imaginary cell array
            rebuilt_cell_im = cell(1, expt_im_length);
            for k = 1:length(ref_im)
                rebuilt_cell_im {k} = reshape(basis_im(:,k), [dim, dim]);
            end
            
            testCase.verify_cell(rebuilt_cell_re, rebuilt_cell_im, keys, ...
                ref_re, ref_im);
        end
        
        function verify_monolith_sparse(testCase, ...
                basis_re, basis_im, keys, ...
                ref_re, ref_im)
            error("Not implemented");
        end
    end
    
    methods (Test)
        function dense_cell_real(testCase)
            sys_id = mtk('imported_matrix_system', 'real');
            m_id = mtk('import_matrix', 'symmetric', ...
                sys_id, testCase.dense_input1);
            
            [re_b, im_b, keys] = mtk('generate_basis', 'cell', 'dense', ...
                sys_id, m_id);
            
            testCase.verify_cell(re_b, im_b, keys, ...
                testCase.dense_expected_re1, ...
                cell(1,0));
        end
        
        function dense_monolith_real(testCase)
            sys_id = mtk('imported_matrix_system', 'real');
            m_id = mtk('import_matrix', 'symmetric', ...
                sys_id, testCase.dense_input1);
            
            [re_b, im_b, keys] = ...
                mtk('generate_basis', 'monolith', 'dense', ...
                sys_id, m_id);
            
            
            testCase.verify_monolith(re_b, im_b, keys, ...
                testCase.dense_expected_re1, ...
                cell(1,0));
        end
        
        function sparse_cell_real(testCase)
            sys_id = mtk('imported_matrix_system', 'real');
            m_id = mtk('import_matrix', 'symmetric', ...
                sys_id, testCase.dense_input1);
            
            [re_b, im_b, keys] = mtk('generate_basis', 'cell', 'sparse', ...
                sys_id, m_id);
            
            testCase.verify_cell(re_b, im_b, keys, ...
                testCase.sparse_expected_re1, ...
                sparse(0,0));
        end
        
        function sparse_monolith_real(testCase)
            sys_id = mtk('imported_matrix_system', 'real');
            m_id = mtk('import_matrix', 'symmetric', ...
                sys_id, testCase.dense_input1);

            [re_b, im_b, keys] = ...
                mtk('generate_basis', 'monolith', 'sparse', ...
                sys_id, m_id);

            testCase.verify_monolith(re_b, im_b, keys, ...
                testCase.sparse_expected_re1, ...
                sparse(0,0));
        end
        
        function dense_cell_complex(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            m_id = mtk('import_matrix', 'hermitian', ...
                sys_id, testCase.string_input2);
            
            [re_b, im_b, keys] = mtk('generate_basis', 'cell', 'dense', ...
                sys_id, m_id);
            
            testCase.verify_cell(re_b, im_b, keys, ...
                testCase.dense_expected_re2, ...
                testCase.dense_expected_im2);
        end
        
        function dense_monolith_complex(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            m_id = mtk('import_matrix', 'hermitian', ...
                sys_id, testCase.string_input2);
            
            [re_b, im_b, keys] = ...
                mtk('generate_basis', 'monolith', 'dense', ...
                sys_id, m_id);
            
            
            testCase.verify_monolith(re_b, im_b, keys, ...
                testCase.dense_expected_re2, ...
                testCase.dense_expected_im2);
        end
        
        function sparse_cell_complex(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            m_id = mtk('import_matrix', 'hermitian', ...
                sys_id, testCase.string_input2);
            
            [re_b, im_b, keys] = mtk('generate_basis', 'cell', 'sparse', ...
                sys_id, m_id);
            
            testCase.verify_cell(re_b, im_b, keys, ...
                testCase.sparse_expected_re2, ...
                testCase.sparse_expected_im2);
        end
        
        function sparse_monolith_complex(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            m_id = mtk('import_matrix', 'hermitian', ...
                sys_id, testCase.string_input2);
            
            [re_b, im_b, keys] = ...
                mtk('generate_basis', 'monolith', 'sparse', ...
                sys_id, m_id);
            
            
            testCase.verify_monolith(re_b, im_b, keys, ...
                testCase.sparse_expected_re2, ...
                testCase.sparse_expected_im2);
        end
    end
    
    
    methods (Test, TestTags={'Error'})
        function Error_NoInput(testCase)
            function no_in()
                [~, ~] = mtk('generate_basis');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooManyInputs(testCase)
            function bad_call()
                sys_id = mtk('imported_matrix_system', 'real');
                m_id = mtk('import_matrix', 'symmetric', ...
                    sys_id, testCase.dense_input1);
                
                [~, ~] = mtk('generate_basis', sys_id, m_id, m_id);
            end
            testCase.verifyError(@() bad_call(), 'mtk:too_many_inputs');
        end
        
        function Error_BadMatrixSystem(testCase)
            function bad_call()
                sys_id = mtk('imported_matrix_system', 'real');
                m_id = mtk('import_matrix', 'symmetric', ...
                    sys_id, testCase.dense_input1);
                
                [~, ~] = mtk('generate_basis', sys_id+1, m_id);
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
        
        
        function Error_BadMatrix(testCase)
            function bad_call()
                sys_id = mtk('imported_matrix_system', 'real');
                m_id = mtk('import_matrix', 'symmetric', ...
                    sys_id, testCase.dense_input1);
                
                [~, ~] = mtk('generate_basis', sys_id, m_id+1);
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
        
        
        function Error_CellAndMonolith(testCase)
            function call_bad()
                sys_id = mtk('imported_matrix_system', 'real');
                m_id = mtk('import_matrix', 'symmetric', ...
                    sys_id, testCase.dense_input1);
                
                [~, ~] = mtk('generate_basis', ...
                    'cell', 'monolith', sys_id, m_id);
            end
            testCase.verifyError(@() call_bad, ...
                'mtk:mutex_param');
        end
        
        function Error_DenseAndSparse(testCase)
            function call_bad()
                sys_id = mtk('imported_matrix_system', 'real');
                m_id = mtk('import_matrix', 'symmetric', ...
                    sys_id, testCase.dense_input1);
                
                [~, ~] = mtk('generate_basis', ...
                    'sparse', 'dense', sys_id, m_id);
            end
            testCase.verifyError(@() call_bad, ...
                'mtk:mutex_param');
        end
    end
end

