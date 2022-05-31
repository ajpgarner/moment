classdef MakeSymmetricTest < matlab.unittest.TestCase
    %GENERATEBASISTEST Unit tests for make_symmetric function
    
    properties(Constant)
        A_input_dense = [[1, 1, 3]; [-2, 2, 4]; [2, 4, 5]];
        A_input_sparse = sparse([[1, 1, 3]; [-2, 2, 4]; [2, 4, 5]]); 
        A_input_string= [["1","1","3"]; ["-2","2","4"]; ["2","4","5"]];
        A_expected_dense = int64([[1, 1, -1]; [1, -1, 4]; [-1, 4, 5]]);
        A_expected_sparse = sparse([[1, 1, -1]; [1, -1, 4]; [-1, 4, 5]]);
        A_expected_subs = [["2", "-1"]; ["3", "-1"]];
        
        B_input_dense = [[1, 2, 0]; [0, 4, 5]; [3, 6, 7]];
        B_input_sparse = sparse([[1, 2, 0]; [0, 4, 5]; [3, 6, 7]]);
        B_input_string = [["1","2","0"]; ["0","4","5"]; ["3","6","7"]];
        B_expected_dense = int64([[1, 0, 0]; [0, 4, 5]; [0, 5, 7]]);
        B_expected_sparse = sparse([[1, 0, 0]; [0, 4, 5]; [0, 5, 7]]);
        B_expected_subs = [["2", "0"]; ["3", "0"]; ["6", "5"]];
        
        C_input_dense = [[1, 2, 0]; [-2, 1, 2]; [3, 4, 5]];
        C_input_sparse = sparse([[1, 2, 0]; [-2, 1, 2]; [3, 4, 5]]);
        C_input_string = [["1","2","0"]; ["-2","1","2"]; ["3","4","5"]];
        C_expected_dense = int64([[1, 0, 0]; [0, 1, 0]; [0, 0, 5]]);
        C_expected_sparse = sparse([[1, 0, 0]; [0, 1, 0]; [0, 0, 5]]);
        C_expected_subs = [["2", "0"]; ["3", "0"]; ["4", "0"]];
    end
    
    
    methods(TestMethodSetup)
        function addNPATKpath(testCase)
            import matlab.unittest.fixtures.PathFixture
            testCase.applyFixture(PathFixture([".."]));
        end
    end
    
    methods(TestMethodTeardown)
        function clearNPATK(testCase)
            clear npatk
        end
    end
      
    methods (Test)
        function Plain_DenseToDense(testCase)
           [actual_dense, actual_subs] = npatk('make_symmetric', ...
                                               'dense', ...
                                                testCase.A_input_dense);
           testCase.verifyEqual(actual_dense, testCase.A_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.A_expected_subs);
        end
        
        function Plain_DenseToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                               testCase.A_input_dense);
           testCase.verifyEqual(actual_sparse, testCase.A_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.A_expected_subs);
        end
        
        function Plain_SparseToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.A_input_sparse);
           testCase.verifyEqual(actual_dense, testCase.A_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.A_expected_subs);
        end
        
        function Plain_SparseToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                               testCase.A_input_sparse);
           testCase.verifyEqual(actual_sparse, testCase.A_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.A_expected_subs);
        end
        
        function Plain_StringToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.A_input_string);
           testCase.verifyEqual(actual_dense, testCase.A_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.A_expected_subs);
        end
        
        function Plain_StringToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                               testCase.A_input_string);
           testCase.verifyEqual(actual_sparse, testCase.A_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.A_expected_subs);
        end
                
        function Zeroes_DenseToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.B_input_dense);
           testCase.verifyEqual(actual_dense, testCase.B_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.B_expected_subs);
        end
        
        function Zeroes_DenseToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                               testCase.B_input_dense);
           testCase.verifyEqual(actual_sparse, testCase.B_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.B_expected_subs);
        end
        
        function Zeroes_SparseToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.B_input_sparse);
           testCase.verifyEqual(actual_dense, testCase.B_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.B_expected_subs);
        end
        
        function Zeroes_SparseToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                               testCase.B_input_sparse);
           testCase.verifyEqual(actual_sparse, testCase.B_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.B_expected_subs);
        end
      
        function Zeroes_StringToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.B_input_string);
           testCase.verifyEqual(actual_dense, testCase.B_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.B_expected_subs);
        end
        
        function Zeroes_StringToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ... 
                                               'sparse', ...
                                               testCase.B_input_string);
           testCase.verifyEqual(actual_sparse, testCase.B_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.B_expected_subs);
        end   
        
        function ResolveToZero_DenseToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.C_input_dense);
           testCase.verifyEqual(actual_dense, testCase.C_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.C_expected_subs);
        end
        
        function ResolveToZero_DenseToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                               testCase.C_input_dense);
           testCase.verifyEqual(actual_sparse, testCase.C_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.C_expected_subs);
        end
        
        function ResolveToZero_SparseToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.C_input_sparse);
           testCase.verifyEqual(actual_dense, testCase.C_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.C_expected_subs);
        end
        
        function ResolveToZero_SparseToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                               testCase.C_input_sparse);
           testCase.verifyEqual(actual_sparse, testCase.C_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.C_expected_subs);
        end
      
        function ResolveToZero_StringToDense(testCase)
           [actual_dense, actual_subs] =npatk('make_symmetric', ...
                                              'dense', ...
                                              testCase.C_input_string);
           testCase.verifyEqual(actual_dense, testCase.C_expected_dense);
           testCase.verifyEqual(actual_subs, testCase.C_expected_subs);
        end
        
        function ResolveToZero_StringToSparse(testCase)
           [actual_sparse, actual_subs] =npatk('make_symmetric', ...
                                               'sparse', ...
                                                testCase.C_input_string);
           testCase.verifyEqual(actual_sparse, testCase.C_expected_sparse);
           testCase.verifyEqual(actual_subs, testCase.C_expected_subs);
        end
                
        function Error_DenseAndSparse(testCase)
            function call_sparse_and_dense()
                [~, ~] = npatk('make_symmetric', 'sparse', 'dense', ...
                               testCase.A_input_string);
            end
           testCase.verifyError(@() call_sparse_and_dense, ...
                                'npatk:mutex_param');           
        end          
    end
end
