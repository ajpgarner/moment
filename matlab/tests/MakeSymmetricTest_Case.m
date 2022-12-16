classdef MakeSymmetricTest_Case
    %MAKESYMMETRICTEST_CASE Subclass, for each example of make_symmetric
    
    properties
        input_dense;
        input_sparse;
        input_string;
        expected_dense;
        expected_sparse;
        expected_subs;
    end
      
    methods
        function obj = MakeSymmetricTest_Case(raw_input, raw_expect, ...
                                              raw_subs)
            obj.input_dense = int64(raw_input);
            obj.input_sparse = sparse(raw_input);
            obj.input_string = string(raw_input);
            obj.expected_dense = int64(raw_expect);
            obj.expected_sparse = sparse(raw_expect);
            obj.expected_subs = raw_subs;
        end
            
        function DenseToDense(testCase, testObj)
           [actual_dense, actual_subs] = mtk('make_symmetric', ...
                                               'dense', ...
                                                testCase.input_dense);
           testObj.verifyEqual(actual_dense, testCase.expected_dense);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
        end
                
        function DenseToSparse(testCase, testObj)
           [actual_sparse, actual_subs] = mtk('make_symmetric', ...
                                                'sparse', ...
                                                testCase.input_dense);
           testObj.verifyEqual(actual_sparse, testCase.expected_sparse);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
        end
        
        function SparseToDense(testCase, testObj)
           [actual_dense, actual_subs] = mtk('make_symmetric', ...
                                               'dense', ...
                                               testCase.input_sparse);
           testObj.verifyEqual(actual_dense, testCase.expected_dense);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
        end
        
        function SparseToSparse(testCase, testObj)
           [actual_sparse, actual_subs] = mtk('make_symmetric', ...
                                                'sparse', ...
                                                testCase.input_sparse);
           testObj.verifyEqual(actual_sparse, testCase.expected_sparse);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
        end
        
        function StringToDense(testCase, testObj)
           [actual_dense, actual_subs] = mtk('make_symmetric', ...
                                               'dense', ...
                                               testCase.input_string);
           testObj.verifyEqual(actual_dense, testCase.expected_dense);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
        end
        
        function StringToSparse(testCase, testObj)
           [actual_sparse, actual_subs] = mtk('make_symmetric', ...
                                                'sparse', ...
                                                testCase.input_string);
           testObj.verifyEqual(actual_sparse, testCase.expected_sparse);
           testObj.verifyEqual(actual_subs, testCase.expected_subs);
        end     
    end
end