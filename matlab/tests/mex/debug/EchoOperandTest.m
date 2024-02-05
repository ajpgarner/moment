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
            requested = mtk('echo_operand', ref_id, uint64(13));
            testCase.verifyEqual(requested, uint64(13));
        end
        
        function ReadRealScalar(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            requested = mtk('echo_operand', ref_id, 13.5);
            testCase.verifyEqual(requested, 13.5);
        end
        
        function ReadComplexScalar(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            requested = mtk('echo_operand', ref_id, 2+1i);
            testCase.verifyEqual(requested, 2+1i);
        end
               
        function ReadRealArray_Dense(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            requested = mtk('echo_operand', ref_id, [1.0, 2.0; 3.0, 4.0]);
            testCase.verifyEqual(requested, [1.0, 2.0; 3.0, 4.0]);
        end
        
        function ReadComplexArray_Dense(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            requested = mtk('echo_operand', ref_id, [1.0, 2+1i; 2-1i, 3.0]);
            testCase.verifyEqual(requested, [1.0, 2+1i; 2-1i, 3.0]);
        end
        
        
        function ReadIdOperatorCell(testCase)
            ms = AlgebraicScenario(3);
            id = ms.id();
            [echoed, is_m] = mtk('echo_operand', ms.System.RefId, ...
                id.OperatorCell);
            testCase.assertTrue(is_m);
            re_mono = MTKMonomial.InitDirect(ms, echoed{:});
            testCase.verifyEqual(id, re_mono);
        end
        
        function ReadVectorIdOperatorCell(testCase)
            ms = AlgebraicScenario(3);
            id = ms.id();
            vec = [id; 5*id];
            [echoed, is_m] = mtk('echo_operand', ms.System.RefId, ...
                vec.OperatorCell);
            testCase.assertTrue(is_m);
            re_mono = MTKMonomial.InitDirect(ms, echoed{:});
            testCase.verifyEqual(vec, re_mono);
        end
        
        
        function ReadMonomialOperatorCell(testCase)
            ms = AlgebraicScenario(3);
            [x, ~, ~] = ms.getAll();
            testCase.assertFalse(x.FoundAllSymbols);
            [echoed, is_m] = mtk('echo_operand', ms.System.RefId, ...
                x.OperatorCell);
            testCase.assertTrue(is_m);
            re_mono = MTKMonomial.InitDirect(ms, echoed{:});
            testCase.verifyEqual(x, re_mono);
        end
        
        function ReadMonomialOperatorCellVector(testCase)
            ms = AlgebraicScenario(3);
            [x, y, ~] = ms.getAll();
            monovec = [x; y];
            testCase.assertFalse(monovec.FoundAllSymbols);
            [echoed, is_m] = mtk('echo_operand', ms.System.RefId, ...
                monovec.OperatorCell);
            testCase.assertTrue(is_m);
            re_mono = MTKMonomial.InitDirect(ms, echoed{:});
            testCase.verifyEqual(re_mono, monovec);
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
        
        function ReadPolynomialOperatorCell2(testCase)
            ms = AlgebraicScenario(3);
            [x, y, z] = ms.getAll();
            poly = 3 + x + y + 2*z;
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