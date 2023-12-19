classdef EchoOperandTest < MTKTestBase
    % ECHOMATRIXTEST Unit tests for echo_operand function.
    % This tests import and export of various algebraic types.
    
    methods (Test, TestTags={'mex'})
        function ReadEmpty(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            requested = mtk('echo_operand', ref_id, {});
            testCase.verifyEqual(requested, false);
        end
        
        function ReadMatrixId(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            requested = mtk('echo_operand', ref_id, 13);
            testCase.verifyEqual(requested, uint64(13));
        end
        
        function ReadPolynomialOperatorCell(testCase)
            ms = AlgebraicScenario(3);
            [x, y, z] = ms.getAll();
            poly = x + y + 2*z;
            testCase.assertFalse(poly.FoundAllSymbols);
            echoed = mtk('echo_operand', ms.System.RefId, poly.OperatorCell);
            re_poly = MTKPolynomial.InitFromOperatorPolySpec(ms, echoed);
            testCase.verifyEqual(poly, re_poly);
        end
        
        function ReadPolynomialOperatorCellVector(testCase)
            ms = AlgebraicScenario(3);
            [x, y, z] = ms.getAll();
            polyvec = [x + y + 2*z; y + 1];
            testCase.assertFalse(polyvec.FoundAllSymbols);
            echoed = mtk('echo_operand', ms.System.RefId, polyvec.OperatorCell);
            re_poly = MTKPolynomial.InitFromOperatorPolySpec(ms, echoed);
            testCase.verifyEqual(re_poly, polyvec);
        end
        
         function ReadPolynomialSymbolCell(testCase)
            ms = AlgebraicScenario(3);
            [~] = ms.WordList(1, true);
            [x, y, z] = ms.getAll();
            poly = x + y + 2*z;
            testCase.assertTrue(poly.FoundAllSymbols);
            echoed = mtk('echo_operand', ms.System.RefId, 'symbolic', poly.SymbolCell);
            re_poly = MTKPolynomial.InitFromOperatorPolySpec(ms, echoed);
            testCase.verifyEqual(poly, re_poly);
        end
        
        function ReadPolynomialSymbolCellVector(testCase)
            ms = AlgebraicScenario(3);
            [~] = ms.WordList(1, true);
            [x, y, z] = ms.getAll();
            polyvec = [x + y + 2*z; y + 1];
            testCase.assertTrue(polyvec.FoundAllSymbols);
            echoed = mtk('echo_operand', ms.System.RefId, 'symbolic', polyvec.SymbolCell);
            re_poly = MTKPolynomial.InitFromOperatorPolySpec(ms, echoed);
            testCase.verifyEqual(re_poly, polyvec);
        end
    end
    
    methods (Test,  TestTags={'Mex', 'Error'})
        function Error_BadMatrixKey1(testCase)
            function bad_echo()
                [~] = mtk('echo_operand', 'bah', 13);
            end
            testCase.verifyError(@() bad_echo(), 'mtk:bad_param');
        end
        function Error_BadMatrixKey2(testCase)
            function bad_echo()
                [~] = mtk('echo_operand', 1234, 13);
            end
            testCase.verifyError(@() bad_echo(), 'mtk:bad_signature');
        end
        function Error_BadMatrixKey3(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            function bad_echo()
                [~] = mtk('echo_operand', ref_id+1, 13);
            end
            testCase.verifyError(@() bad_echo(), 'mtk:bad_param');
        end
    end
end