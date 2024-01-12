classdef MultiplyTest < MTKTestBase
    %SIMPLIFYTEST Unit tests for `multiply` mtk function
    
    methods (Test, TestTags={'mex'})
        function MonomialMonomial(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            [res, is_mono] = ...
                mtk('multiply', ref_id, x.OperatorCell, y.OperatorCell);
            testCase.assertTrue(is_mono);
            res_mono = MTKMonomial.InitDirect(scenario, res{:});
            testCase.assertEqual(size(res_mono), [1 1]);
            testCase.verifyEqual(res_mono.Operators, uint64([1 2]));
        end
        
        function MonomialPolynomial(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            direct_x_plus_y = MTKPolynomial(scenario, [x; y]);
            [res, is_mono] = mtk('multiply', ref_id, ...
                x.OperatorCell, ...
                direct_x_plus_y.OperatorCell);
            
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [1 1]);
            testCase.verifyEqual(res_poly.Constituents.Operators, ...
                {uint64([1 1]); uint64([1 2])});
        end
        
        function PolynomialMonomial(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            direct_x_plus_y = MTKPolynomial(scenario, [x; y]);
            [res, is_mono] = mtk('multiply', ref_id, ...
                direct_x_plus_y.OperatorCell, ...
                x.OperatorCell);
            
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [1 1]);
            testCase.verifyEqual(res_poly.Constituents.Operators, ...
                {uint64([1 1]); uint64([2 1])});
        end
        
        function PolynomialPolynomial(testCase)
            scenario = AlgebraicScenario(3);
            ref_id = scenario.System.RefId;
            [x, y, z] = scenario.getAll();
            direct_x_plus_y = MTKPolynomial(scenario, [x; y]);
            direct_y_plus_z = MTKPolynomial(scenario, [y; z]);
            [res, is_mono] = mtk('multiply', ref_id, ...
                direct_x_plus_y.OperatorCell, ...
                direct_y_plus_z.OperatorCell);
            
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [1 1]);
            testCase.verifyEqual(res_poly.Constituents.Operators, ...
                {uint64([1 2]); uint64([1 3]); uint64([2 2]); uint64([2 3])});
            testCase.verifyEqual(res_poly.Constituents.Coefficient, ...
                complex([1.0; 1.0; 1.0; 1.0]));
        end
        
        
        function PolynomialPolynomial_Binomial(testCase)
            scenario = AlgebraicScenario(3);
            ref_id = scenario.System.RefId;
            id = scenario.id();
            [x, ~, ~] = scenario.getAll();
            direct_poly = MTKPolynomial(scenario, [id; x]);
            [res, is_mono] = mtk('multiply', ref_id, ...
                direct_poly.OperatorCell, ...
                direct_poly.OperatorCell);
            
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [1 1]);
            testCase.verifyEqual(res_poly.Constituents.Operators, ...
                {uint64.empty(1,0); uint64([1]); uint64([1 1])});
            testCase.verifyEqual(res_poly.Constituents.Coefficient, ...
                complex([1.0; 2.0; 1.0]));
        end
        
        
        function MonomialScalarArray(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            stack = [x; y];
            [res, is_mono] = ...
                mtk('multiply', ref_id, x.OperatorCell, stack.OperatorCell);
            testCase.assertTrue(is_mono);
            res_poly = MTKMonomial.InitDirect(scenario, res{:});
            testCase.assertEqual(size(res_poly), [2 1]);            
            testCase.verifyEqual(res_poly.Operators, ...
                                 {uint64([1 1]); uint64([1 2])});
            testCase.verifyEqual(res_poly.Coefficient, complex([1.0; 1.0]));            
        end
        
        
        function MonomialArrayScalar(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            stack = [x; y];
            [res, is_mono] = ...
                mtk('multiply', ref_id, stack.OperatorCell, x.OperatorCell);
            testCase.assertTrue(is_mono);
            res_poly = MTKMonomial.InitDirect(scenario, res{:});
            testCase.assertEqual(size(res_poly), [2 1]);            
            testCase.verifyEqual(res_poly.Operators, ...
                                 {uint64([1 1]); uint64([2 1])});
            testCase.verifyEqual(res_poly.Coefficient, complex([1.0; 1.0]));            
        end
        
        function MonomialArrayArray(testCase)
            scenario = AlgebraicScenario(3);
            ref_id = scenario.System.RefId;
            
            [x, y, z] = scenario.getAll();
            lhs_stack = [x; y];
            rhs_stack = [y; z];
            
            [res, is_mono] =  mtk('multiply', ref_id, ...
                lhs_stack.OperatorCell, ...
                rhs_stack.OperatorCell);
            testCase.assertTrue(is_mono);
            res_poly = MTKMonomial.InitDirect(scenario, res{:});
            testCase.assertEqual(size(res_poly), [2 1]);            
            testCase.verifyEqual(res_poly.Operators, ...
                                 {uint64([1 2]); uint64([2 3])});
            testCase.verifyEqual(res_poly.Coefficient, complex([1.0; 1.0]));   
        end
                
        function ScalarMatrix(testCase)
            scenario = AlgebraicScenario(3);
            ref_id = scenario.System.RefId;
            
            mm = scenario.MomentMatrix(1);
            [x, ~, ~] = scenario.getAll();
            
            [id, dim, is_mono, is_herm] = ...
                mtk('multiply', ref_id, x.OperatorCell, mm.Index);
            testCase.verifyEqual(id, int64(1))
            testCase.verifyEqual(dim, uint64(4));
            testCase.verifyTrue(is_mono);
            testCase.verifyFalse(is_herm);
            ss = mtk('operator_matrix', 'sequence_string', ref_id, id);
            testCase.verifyEqual(ss(3,1), "<X1;X2>");
        end
        
        function MatrixScalar(testCase)
            scenario = AlgebraicScenario(3);
            ref_id = scenario.System.RefId;
            
            mm = scenario.MomentMatrix(1);
            [x, ~, ~] = scenario.getAll();
            
            [id, dim, is_mono, is_herm] = ...
                mtk('multiply', ref_id, mm.Index, x.OperatorCell);
            testCase.verifyEqual(id, int64(1))
            testCase.verifyEqual(dim, uint64(4));
            testCase.verifyTrue(is_mono);
            testCase.verifyFalse(is_herm);
            
            ss = mtk('operator_matrix', 'sequence_string', ref_id, id);
            testCase.verifyEqual(ss(3,1), "<X2;X1>");
        end        
    end
end
