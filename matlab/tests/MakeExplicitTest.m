classdef MakeExplicitTest < MTKTestBase
    %MAKEEXPLICITTEST Unit tests for make_explicit function
    methods (Test)
        function Inflation_SimplePair(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2 2], {}, 1);
            [~] = mtk('moment_matrix', ref_id, 1);
            symbols = mtk('symbol_table', ref_id, {[1], [2], [1 2]});
            [id_A, id_B, id_AB] = symbols.symbol;
            
            explicit = mtk('make_explicit', ref_id, [0.1 0.2 0.3 0.4]);
            expected = {{id_A, 0.3}, {id_B, 0.4}, {id_AB, 0.1}};
            testCase.verifyEqual(explicit, expected, 'RelTol', 1e-12);
        end
        
        function Inflation_PairFromTriangle(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2 2 2], {}, 1);
            [~] = mtk('moment_matrix', ref_id, 1);
            ABsymbols = mtk('symbol_table', ref_id, ...
                {[1], [2], [3], [1 2], [1 3]});
            [id_A, id_B, id_C, id_AB, id_AC] = ABsymbols.symbol;
            
            explicitAB = mtk('make_explicit', ref_id, ...
                [1; 2], [0.1 0.2 0.3 0.4]);
            expectedAB = {{id_A, 0.3}, {id_B, 0.4}, {id_AB, 0.1}};
            testCase.verifyEqual(explicitAB, expectedAB, 'RelTol', 1e-12);
            
            explicitAC = mtk('make_explicit', ref_id, ...
                [1; 3], [0.1 0.2 0.3 0.4]);
            expectedAC = {{id_A, 0.3}, {id_C, 0.4}, {id_AC, 0.1}};
            testCase.verifyEqual(explicitAC, expectedAC, 'RelTol', 1e-12);
        end
        
        function Inflation_SimpleTriangle(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2 2 2], {}, 1);
            [~] = mtk('moment_matrix', ref_id, 2);
            symbols = mtk('symbol_table', ref_id, ...
                {[1], [2], [3], [1 2], [1 3], [2 3], [1 2 3]});
            [id_A, id_B, id_C, id_AB, id_AC, id_BC, id_ABC] = symbols.symbol;
            
            explicit = mtk('make_explicit', ref_id, [0.5 0 0 0 0 0 0 0.5]);
            expected = {{id_A, 0.5}, {id_B, 0.5}, {id_C, 0.5}, ...
                {id_AB, 0.5}, {id_AC, 0.5}, {id_BC, 0.5}, ...
                {id_ABC, 0.5}};
            testCase.verifyEqual(explicit, expected, 'RelTol', 1e-12);
        end
        
        function Inflation_TriangleWithVariants(testCase)
            ref_id = mtk('new_inflation_matrix_system', [2 2 2], ...
                {[1, 2], [1, 3], [2, 3]}, 2);
            [~] = mtk('moment_matrix', ref_id, 2);
            symbols = mtk('symbol_table', ref_id, {[1], [5], [1 5]});
            [id_A, id_B, id_AB] = symbols.symbol;
            
            explicit = mtk('make_explicit', ref_id, [1, 1; 2, 1], ...
                [0.1 0.2 0.3 0.4]);
            expected = {{id_A, 0.3}, {id_B, 0.4}, {id_AB, 0.1}};
            testCase.verifyEqual(explicit, expected, 'RelTol', 1e-12);
        end
           
        function Locality_Pairs(testCase)
            ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            symbols = mtk('symbol_table', ref_id, ...
                {[1], [2], [3], [4], [1 3], [1 4], [2 3], [2 4]});
            [a0, a1, b0, b1, a0b0, a0b1, a1b0, a1b1] = symbols.symbol;
            
            explicit_00 = mtk('make_explicit', ref_id, [1, 1; 2, 1], ...
                              [0.1 0.2 0.3 0.4]);
            expected_00 = {{a0, 0.3}, {b0, 0.4}, {a0b0, 0.1}};
            testCase.verifyEqual(explicit_00, expected_00, 'RelTol', 1e-12);
            
            explicit_01 = mtk('make_explicit', ref_id, [1, 1; 2, 2], ...
                              [0.1 0.2 0.3 0.4]);
            expected_01 = {{a0, 0.3}, {b1, 0.4}, {a0b1, 0.1}};
            testCase.verifyEqual(explicit_01, expected_01, 'RelTol', 1e-12);
            
            explicit_10 = mtk('make_explicit', ref_id, [1, 2; 2, 1], ...
                              [0.1 0.2 0.3 0.4]);
            expected_10 = {{a1, 0.3}, {b0, 0.4}, {a1b0, 0.1}};
            testCase.verifyEqual(explicit_10, expected_10, 'RelTol', 1e-12);
            
            explicit_11 = mtk('make_explicit', ref_id, [1, 2; 2, 2], ...
                              [0.1 0.2 0.3 0.4]);
            expected_11 = {{a1, 0.3}, {b1, 0.4}, {a1b1, 0.1}};
            testCase.verifyEqual(explicit_11, expected_11, 'RelTol', 1e-12);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('make_explicit');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()
                ref_id = mtk('new_inflation_matrix_system', [2 2], {}, 1);
                [~] = mtk('moment_matrix', ref_id, 1);
                [~] = mtk('make_explicit', ref_id);
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_BadValues(testCase)
            function no_in()
                ref_id = mtk('new_inflation_matrix_system', [2 2], {}, 1);
                [~] = mtk('moment_matrix', ref_id, 1);
                [~] = mtk('make_explicit', ref_id, [0 0 0 0 1]);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end 
        
        function Error_BadMmts(testCase)
            function no_in()
                ref_id = mtk('new_inflation_matrix_system', [2 2], {}, 1);
                [~] = mtk('moment_matrix', ref_id, 1);
                [~] = mtk('make_explicit', ref_id, [1 3], [0 0 0 1]);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end 
    end
end
