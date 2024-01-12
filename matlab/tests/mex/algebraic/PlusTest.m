classdef PlusTest < MTKTestBase
%SIMPLIFYTEST Unit tests for `plus` mtk function
    
    methods (Test, TestTags={'mex'})
        function MonomialMonomial(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            [res, is_mono] = ...
                mtk('plus', ref_id, x.OperatorCell, y.OperatorCell);
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.verifyEqual(size(res_poly), [1 1]);
            testCase.verifyEqual(res_poly.Constituents.Operators, ...
                    {uint64([1]); uint64([2])});
        end
        
        function MonomialPolynomial(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            direct_x_plus_y = MTKPolynomial(scenario, [x; y]);
            [res, is_mono] = mtk('plus', ref_id, ...
                                 x.OperatorCell, ...
                                 direct_x_plus_y.OperatorCell);
                             
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [1 1]);
            testCase.verifyEqual(res_poly.Constituents.Operators, ...
                    {uint64([1]); uint64([2])});
        end
        
       function ScalarArray(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            stack = [x; y];
            [res, is_mono] = ...
                mtk('plus', ref_id, x.OperatorCell, stack.OperatorCell);
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [2 1]);
            top_poly = res_poly.Constituents{1};            
            testCase.verifyEqual(top_poly.Operators, uint64(1));
            testCase.verifyEqual(top_poly.Coefficient, complex(2.0));
            bottom_poly = res_poly.Constituents{2};
            testCase.verifyEqual(bottom_poly.Operators, ...
                                 {uint64([1]); uint64([2])});
            testCase.verifyEqual(bottom_poly.Coefficient, ...
                                 complex([1.0; 1.0]));
       end
        
       function ArrayScalar(testCase)
            scenario = AlgebraicScenario(2);
            ref_id = scenario.System.RefId;
            [x, y] = scenario.getAll();
            stack = [x; y];
            [res, is_mono] = ...
                mtk('plus', ref_id, stack.OperatorCell, x.OperatorCell);
            testCase.assertFalse(is_mono);
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [2 1]);
            top_poly = res_poly.Constituents{1};            
            testCase.verifyEqual(top_poly.Operators, uint64(1));
            testCase.verifyEqual(top_poly.Coefficient, complex(2.0));
            bottom_poly = res_poly.Constituents{2};
            testCase.verifyEqual(bottom_poly.Operators, ...
                                 {uint64([1]); uint64([2])});
            testCase.verifyEqual(bottom_poly.Coefficient, ...
                                 complex([1.0; 1.0]));
       end

       function ArrayArray(testCase)
            scenario = AlgebraicScenario(3);
            ref_id = scenario.System.RefId;
            
            [x, y, z] = scenario.getAll();
            lhs_stack = [x; y];
            rhs_stack = [y; z];
            
            [res, is_mono] =  mtk('plus', ref_id, ...
                                  lhs_stack.OperatorCell, ...
                                  rhs_stack.OperatorCell);
            testCase.assertFalse(is_mono);
            
            res_poly = MTKPolynomial.InitFromOperatorPolySpec(scenario, res);
            testCase.assertEqual(size(res_poly), [2 1]);
            top_poly = res_poly.Constituents{1};
            testCase.verifyEqual(top_poly.Operators, ...
                                 {uint64([1]); uint64([2])});
            testCase.verifyEqual(top_poly.Coefficient, ...
                                 complex([1.0; 1.0]));
            bottom_poly = res_poly.Constituents{2};
            testCase.verifyEqual(bottom_poly.Operators, ...
                                 {uint64([2]); uint64([3])});
            testCase.verifyEqual(bottom_poly.Coefficient, ...
                                 complex([1.0; 1.0]));
       end
        
       function ScalarMatrix(testCase)
           scenario = AlgebraicScenario(3);
           ref_id = scenario.System.RefId;
           
           mm = scenario.MomentMatrix(1);
           [x, ~, ~] = scenario.getAll();
           
           [id, dim, is_mono, is_herm] = ...
               mtk('plus', ref_id, mm.Index, x.OperatorCell);
           testCase.verifyEqual(id, int64(1))
           testCase.verifyEqual(dim, uint64(4));
           testCase.verifyFalse(is_mono);
           testCase.verifyTrue(is_herm);
       end
       
       
       function MatrixScalar(testCase)
           scenario = AlgebraicScenario(3);
           ref_id = scenario.System.RefId;
           
           mm = scenario.MomentMatrix(1);
           [x, ~, ~] = scenario.getAll();
           
           [id, dim, is_mono, is_herm] = ...
               mtk('plus', ref_id, x.OperatorCell, mm.Index);
           testCase.verifyEqual(id, int64(1))
           testCase.verifyEqual(dim, uint64(4));
           testCase.verifyFalse(is_mono);
           testCase.verifyTrue(is_herm);
       end
       
       function MatrixMatrix(testCase)
           scenario = AlgebraicScenario(3);
           ref_id = scenario.System.RefId;
           
           mm = scenario.MomentMatrix(1);
           [x, ~, ~] = scenario.getAll();
           lmX = scenario.LocalizingMatrix(x, 1);
           
           [id, dim, is_mono, is_herm] = ...
               mtk('plus', ref_id, x.OperatorCell, mm.Index);
           testCase.verifyEqual(id, int64(2))
           testCase.verifyEqual(dim, uint64(4));
           testCase.verifyFalse(is_mono);
           testCase.verifyTrue(is_herm);
       end
    end    
end
