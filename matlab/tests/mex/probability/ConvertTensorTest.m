classdef ConvertTensorTest < MTKTestBase
%CONVERTTESNOR Unit tests for convert_tensor function.
    
    methods (Test, TestTags={'mex'})
        function Bipartite11_ToCG(testCase)
            input_fc = [0.0, -1.0; -1.0, 1.0];
            expected_cg =  [3.0, -4.0; -4.0, 4.0];
            actual_cg = mtk('convert_tensor', 'fc2cg', input_fc);
            testCase.verifyEqual(actual_cg, expected_cg);
        end
        
        function Bipartite11_ToFC(testCase)
            input_cg = [3.0, -4.0; -4.0, 4.0];
            expected_fc = [0.0, -1.0; -1.0, 1.0];
            actual_fc = mtk('convert_tensor', 'cg2fc', input_cg);
            testCase.verifyEqual(actual_fc, expected_fc);
        end
        
        function CHSH_ToCG(testCase)
            input_fc = [[0 0 0]; [0 1 1]; [0 1 -1]];
            expected_cg = [[2 -4 0]; [-4 4 4]; [0 4 -4]];
            actual_cg = mtk('convert_tensor', 'fc2cg', input_fc);
            testCase.verifyEqual(actual_cg, expected_cg);
        end
        
        function CHSH_ToFC(testCase)
            input_cg = [[2 -4 0]; [-4 4 4]; [0 4 -4]];
            expected_fc = [[0 0 0]; [0 1 1]; [0 1 -1]];
            actual_fc = mtk('convert_tensor', 'cg2fc', input_cg);
            testCase.verifyEqual(actual_fc, expected_fc);
        end       
        
        function Bipartite32_ToCG(testCase)                                         
            input_fc = [[0 0 0]; [0 1 0]; [0 0 1]; [0 -1 0]];
            expected_cg = [[1 0 -2]; [-2 4 0]; [-2 0 4]; [2 -4 0]];
            actual_cg = mtk('convert_tensor', 'fc2cg', input_fc);
            testCase.verifyEqual(actual_cg, expected_cg);
        end
        
        function Bipartite32_ToFC(testCase)
            input_cg =[[1 0 -2]; [-2 4 0]; [-2 0 4]; [2 -4 0]];
            expected_fc = [[0 0 0]; [0 1 0]; [0 0 1]; [0 -1 0]];
            actual_fc = mtk('convert_tensor', 'cg2fc', input_cg);
            testCase.verifyEqual(actual_fc, expected_fc);
        end
    end
end
