classdef EchoTest < MTKTestBase
% ECHOTEST Unit tests for echo function.
% Particularly, this tests import and export between MATLAB and Eigen-type
% matrices.
    methods (Test)
        function DenseDouble_To_Dense(testCase)
            A = [[1, 2, 0, 10]; [3, 4, 5, 11]; [6, 7, 0, 12]];
            actual = mtk('echo', 'dense', A);            
            testCase.verifyEqual(actual, A);            
        end
        
        function Int64_To_Dense(testCase)
            A = [[1, 2, 0]; [3, 4, 5]; [6, 7, 0]];
            intA = int64(A);
            actual = mtk('echo', 'dense', intA);            
            testCase.verifyEqual(actual, A);            
        end
            
        function Sparse_To_Dense(testCase)
            A = [[1, 2, 0, 10]; [3, 4, 5, 11]; [6, 7, 0, 12]];
            sparseA = sparse(A);
            actual = mtk('echo', 'dense', sparseA);            
            testCase.verifyEqual(actual, A);            
        end
            
        function String_To_Dense(testCase)
            A = [["1", "2", "0", "10"]; 
                 ["3", "4", "5", "-11"];
                 ["6", "7", "0", "12"]];
            expected = [[1, 2, 0, 10]; [3, 4, 5, -11]; [6, 7, 0, 12]];
            actual = mtk('echo', 'dense', A);            
            testCase.verifyEqual(actual, expected);            
        end
        
        function DenseDouble_To_Sparse(testCase)
            A = [[1, 2, 0, 10]; [3, 4, 5, 11]; [6, 7, 0, 12]];
            expected = sparse(A);
            actual = mtk('echo', 'sparse', A);            
            testCase.verifyEqual(actual, expected);            
        end
        
        function Int64_To_Sparse(testCase)
            A = [[1, 2, 0]; [3, 4, 5]; [6, 7, 0]];
            intA = int64(A);
            expected = sparse(A);
            actual = mtk('echo', 'sparse', intA);            
            testCase.verifyEqual(actual, expected);            
        end
            
        function Sparse_To_Sparse(testCase)
            A = [[1, 2, 0, 10]; [3, 4, 5, 11]; [6, 7, 0, 12]];
            expected = sparse(A);
            actual = mtk('echo', 'sparse', expected);            
            testCase.verifyEqual(actual, expected);            
        end
            
        function String_To_Sparse(testCase)
            A = [["1", "2", "0", "10"]; 
                 ["3", "4", "5", "-11"];
                 ["6", "7", "0", "12"]];
            expected = sparse([[1, 2, 0, 10]; [3, 4, 5, -11]; [6, 7, 0, 12]]);
            actual = mtk('echo', 'sparse', A);            
            testCase.verifyEqual(actual, expected);            
        end
    end
end