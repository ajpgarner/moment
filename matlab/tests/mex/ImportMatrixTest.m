classdef ImportMatrixTest < MTKTestBase
    %IMPORTMATRIXTEST Unit tests for import_matrix function
    methods (Test, TestTags={'mex'})
        function RealSystem_RealMatrix(testCase)
            sys_id = mtk('imported_matrix_system', 'real');
            A = [[1, 2, 3]; [4, 5, 6]; [7, 8, 9]];
            m_id = mtk('import_matrix', 'real', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["4","5","6"]; ["7","8","9"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 10);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6 7 8 9]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6 7 8 9]));
        end
        
        function RealSystem_RealMatrixString(testCase)
            sys_id = mtk('imported_matrix_system', 'real');
            A = [["1","2","3"]; ["4","5","6"]; ["7","8","9"]];
            m_id = mtk('import_matrix', 'real', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["4","5","6"]; ["7","8","9"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 10);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6 7 8 9]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6 7 8 9]));
        end
        
        function RealSystem_SymmetricMatrix(testCase)
            sys_id = mtk('imported_matrix_system', 'real');
            A = [[1, 2, 3]; [2, 4, 5]; [3, 5, 6]];
            m_id = mtk('import_matrix', 'symmetric', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["2","4","5"]; ["3","5","6"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 7);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6]));
        end
        
        function ComplexSystem_RealMatrix(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            A = [[1, 2, 3]; [4, 5, 6]; [7, 8, 9]];
            m_id = mtk('import_matrix', 'real', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["4","5","6"]; ["7","8","9"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 10);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6 7 8 9]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6 7 8 9]));
            testCase.verifyEqual([symbol_table.basis_im], ...
                uint64([0 0 0 0 0 0 0 0 0 0]));
        end
        
        function ComplexSystem_ComplexMatrix(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            A = [[1, 2, 3]; [4, 5, 6]; [7, 8, 9]];
            m_id = mtk('import_matrix', 'complex', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["4","5","6"]; ["7","8","9"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 10);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6 7 8 9]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6 7 8 9]));
            testCase.verifyEqual([symbol_table.basis_im], ...
                uint64([0 0 1 2 3 4 5 6 7 8])); % Reserved symbol 1 is real
        end
        
        
        function ComplexSystem_SymmetricMatrix(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            A = [[1, 2, 3]; [2, 4, 5]; [3, 5, 6]];
            m_id = mtk('import_matrix', 'symmetric', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["2","4","5"]; ["3","5","6"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 7);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6]));
            testCase.verifyEqual([symbol_table.basis_im], ...
                uint64([0 0 0 0 0 0 0]));
        end
        
        function ComplexSystem_HermitianMatrix(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            A = [["1","2","3"]; ["2*","4","5"]; ["3*","5*","6"]];
            m_id = mtk('import_matrix', 'hermitian', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["2*","4","5"]; ["3*","5*","6"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 7);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6]));
            testCase.verifyEqual([symbol_table.basis_im], ...
                uint64([0 0 1 2 0 3 0]));
        end
        
        function ComplexSystem_RealHermitianMatrix(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            A = [["1","2","3"]; ["2","4","5"]; ["3","5","6"]];
            m_id = mtk('import_matrix', 'hermitian', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2","3"]; ["2","4","5"]; ["3","5","6"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 7);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3 4 5 6]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3 4 5 6]));
            testCase.verifyEqual([symbol_table.basis_im], ...
                uint64([0 0 0 0 0 0 0]));
        end
        
        function ComplexSystem_ReassessRealness(testCase)
            sys_id = mtk('imported_matrix_system', 'complex');
            A = [["1","2"]; ["2*","3"]];
            m_id = mtk('import_matrix', 'hermitian', sys_id, A);
            actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            expected_A = [["1","2"]; ["2*","3"]];
            testCase.verifyEqual(actual_A, expected_A);
            
            symbol_table = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(symbol_table), 4);
            testCase.verifyEqual([symbol_table.symbol], ...
                int64([0 1 2 3]));
            testCase.verifyEqual([symbol_table.basis_re], ...
                uint64([0 1 2 3]));
            testCase.verifyEqual([symbol_table.basis_im], ...
                uint64([0 0 1 0]));
            
            B = [["3","2"]; ["2","4"]];
            mB_id = mtk('import_matrix', 'hermitian', sys_id, B);
            actual_B = mtk('operator_matrix', 'symbol_string', sys_id, mB_id);
            expected_B = [["3","2"]; ["2","4"]];
            testCase.verifyEqual(actual_B, expected_B)
            
            updated_symbols = mtk('symbol_table', sys_id);
            testCase.assertEqual(length(updated_symbols), 5);
            testCase.verifyEqual([updated_symbols.symbol], ...
                int64([0 1 2 3 4]));
            testCase.verifyEqual([updated_symbols.basis_re], ...
                uint64([0 1 2 3 4]));
            testCase.verifyEqual([updated_symbols.basis_im], ...
                uint64([0 0 0 0 0]));
            
            new_actual_A = mtk('operator_matrix', 'symbol_string', sys_id, m_id);
            new_expected_A = [["1","2"]; ["2","3"]];
            testCase.verifyEqual(new_actual_A, new_expected_A);
        end
    end
        
    methods (Test, TestTags={'Mex', 'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('import_matrix');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()
                
                sys_id = mtk('imported_matrix_system', 'real');
                [~] = mtk('import_matrix', sys_id);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
                
        function Error_TooManyInputs(testCase)
            function no_in()                
                A = [[1, 2, 3]; [4, 5, 6]; [7, 8, 9]];
                sys_id = mtk('imported_matrix_system', 'real');
                [~] = mtk('import_matrix', sys_id, A, A);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_many_inputs');
        end
        
        function Error_BadShape(testCase)
            function no_in()
                
                sys_id = mtk('imported_matrix_system', 'real');
                [~] = mtk('import_matrix', sys_id, [[1 2]; [3 4]; [5 6]]);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_NotSymmetric(testCase)
            function no_in()
                
                sys_id = mtk('imported_matrix_system', 'real');
                A = [[1, 2, 3]; [4, 5, 6]; [7, 8, 9]];
                [~] = mtk('import_matrix', 'symmetric', sys_id, A);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_NotHermitian(testCase)
            function no_in()
                sys_id = mtk('imported_matrix_system', 'complex');
                A = [[1, 2, 3]; [4, 5, 6]; [7, 8, 9]];
                [~] = mtk('import_matrix', 'hermitian', sys_id, A);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
    end
end
