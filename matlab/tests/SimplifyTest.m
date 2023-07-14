classdef SimplifyTest < MTKTestBase
%SIMPLIFYTEST Unit tests for simplify function
    
    methods (Test)
        function Identity(testCase)
            ms_id = mtk('algebraic_matrix_system', 2);
            input = uint64.empty(1,0);
            output = mtk('simplify', ms_id, input);
            expected = uint64.empty(1,0);
            testCase.verifyEqual(output, expected);
        end
        
        function FundamentalOperator(testCase)
            ms_id = mtk('algebraic_matrix_system', 2);
            input = uint64([2]);
            output = mtk('simplify', ms_id, input);
            expected = uint64([2]);
            testCase.verifyEqual(output, expected);
        end
        
        function SimpleString(testCase)
            ms_id = mtk('algebraic_matrix_system', 2);
            input = uint64([1, 2]);
            output = mtk('simplify', ms_id, input);
            expected = uint64([1, 2]);
            testCase.verifyEqual(output, expected);
        end
        
        function StringWithRewrite(testCase)
            ms_id = mtk('algebraic_matrix_system', 2, 'nonhermitian', ...
                        {{[1, 1], [2]}}, 'quiet');
            input = uint64([1, 1, 2]);
            output = mtk('simplify', ms_id, input);
            expected = uint64([2, 2]);
            testCase.verifyEqual(output, expected);
        end
        
        function EchoNamed(testCase)
            ms_id = mtk('algebraic_matrix_system', ["A", "B"]);
            input = ["A"];
            output = mtk('simplify', ms_id, input);
            expected = uint64([1]);
            testCase.verifyEqual(output, expected);
        end
        
        function NamedWithRewrite(testCase)
            ms_id = mtk('algebraic_matrix_system', ["A", "B"], 'nonhermitian', ...
                        {{["A", "A"], ["B"]}}, 'quiet');
            input = ["A", "A"];
            output = mtk('simplify', ms_id, input);
            expected = uint64([2]); % = "B"
            testCase.verifyEqual(output, expected);
        end
        
             
        function  WithHash(testCase)
            ms_id = mtk('algebraic_matrix_system', 2);
            input = uint64([2]);
            [output, ~, output_hash] = mtk('simplify', ms_id, input);
            expected = uint64([2]);
            testCase.verifyEqual(output, expected);
            testCase.verifyEqual(output_hash, Util.shortlex_hash(2, expected));
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('simplify');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()
                ms_id = mtk('algebraic_matrix_system', 2);
                [~] = mtk('simplify', ms_id);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_BadMS(testCase)
            function no_in()
                ms_id = mtk('algebraic_matrix_system', 2);
                [~] = mtk('simplify', ms_id+1, [1 2]);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
         function Error_BadOperators(testCase)
            function no_in()
                ms_id = mtk('algebraic_matrix_system', 2);
                [~] = mtk('simplify', ms_id, [3 3]);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
         end
        
         
         function Error_BadNamedOperators(testCase)
            function no_in()
                ms_id = mtk('algebraic_matrix_system', ["A", "B"]);
                [~] = mtk('simplify', ms_id, ["A", "C"]);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
    end
end
