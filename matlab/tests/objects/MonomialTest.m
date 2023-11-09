classdef MonomialTest < MTKTestBase
%MONOMIALTEST Tests for MTKMonomial.

%% Construction and symbol look-up
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'construct'})
    function construct_x(testCase)
        setting = AlgebraicScenario(2);
        x = MTKMonomial(setting, [1], -2.0);
        
        testCase.assertEqual(size(x), [1 1]);
        testCase.assertEqual(numel(x), 1);
        testCase.assertTrue(x.IsScalar);
        testCase.verifyEqual(x.Operators, uint64(1));
        testCase.verifyEqual(x.Coefficient, -2.0);
        testCase.verifyEqual(x.Hash, uint64(2));
        testCase.verifyFalse(x.FoundSymbol);
        testCase.verifyFalse(x.IsZero);
        
        setting.WordList(1, true); % Build symbols
        
        testCase.verifyTrue(x.FoundSymbol);
        testCase.verifyEqual(x.SymbolId, int64(2));
        testCase.verifyEqual(x.RealBasisIndex, uint64(2));
        testCase.verifyEqual(x.ImaginaryBasisIndex, uint64(0));
    end
    
    function construct_xy(testCase)
        setting = AlgebraicScenario(2);
        xy = MTKMonomial(setting, [1, 2], 1.0 - 3.0i);
        
        testCase.assertEqual(size(xy), [1 1]);
        testCase.assertEqual(numel(xy), 1);
        testCase.assertTrue(xy.IsScalar);
        testCase.verifyEqual(xy.Operators, uint64([1, 2]));
        testCase.verifyEqual(xy.Coefficient, 1.0 - 3.0i);
        testCase.verifyEqual(xy.Hash, uint64(5));
        testCase.verifyFalse(xy.FoundSymbol);
        testCase.verifyFalse(xy.IsZero);
        
        setting.WordList(2, true); % Build and register symbols
        
        testCase.verifyTrue(xy.FoundSymbol);
        testCase.verifyEqual(xy.SymbolId, int64(5));
        testCase.verifyEqual(xy.RealBasisIndex, uint64(5));
        testCase.verifyEqual(xy.ImaginaryBasisIndex, uint64(1));
    end
    
    function construct_row_vector(testCase)
        setting = AlgebraicScenario(2);
        x_y = MTKMonomial(setting, {[1], [2]}, 10.0);
        
        testCase.assertEqual(size(x_y), [1 2]);
        testCase.assertEqual(numel(x_y), 2);
        testCase.assertFalse(x_y.IsScalar);
        testCase.assertTrue(x_y.IsVector);
        testCase.assertTrue(x_y.IsRowVector);
        testCase.verifyEqual(x_y.Operators, {uint64(1), uint64(2)});
        testCase.verifyEqual(x_y.Coefficient, [10, 10]);
        testCase.verifyEqual(x_y.Hash, uint64([2, 3]));
        testCase.verifyEqual(x_y.FoundSymbol, [false false]);
        testCase.verifyEqual(x_y.IsZero, [false false]);
        
        setting.WordList(1, true); % Build and register symbols
        testCase.verifyEqual(x_y.FoundSymbol, [true, true]);
        testCase.verifyEqual(x_y.SymbolId, int64([2 3]));
        testCase.verifyEqual(x_y.RealBasisIndex, uint64([2 3]));
        testCase.verifyEqual(x_y.ImaginaryBasisIndex, uint64([0 0]));
    end
    
    function construct_col_vector(testCase)
        setting = AlgebraicScenario(2);
        x_y = MTKMonomial(setting, {[1]; [2]}, 10.0);
        
        testCase.assertEqual(size(x_y), [2 1]);
        testCase.assertEqual(numel(x_y), 2);
        testCase.assertFalse(x_y.IsScalar);
        testCase.assertTrue(x_y.IsVector);
        testCase.assertTrue(x_y.IsColVector);
        testCase.verifyEqual(x_y.Operators, {uint64(1); uint64(2)});
        testCase.verifyEqual(x_y.Coefficient, [10; 10]);
        testCase.verifyEqual(x_y.Hash, uint64([2; 3]));
        testCase.verifyEqual(x_y.FoundSymbol, [false; false]);
        testCase.verifyEqual(x_y.IsZero, [false; false]);
        
        setting.WordList(1, true); % Build and register symbols
        testCase.verifyEqual(x_y.FoundSymbol, [true; true]);
        testCase.verifyEqual(x_y.SymbolId, int64([2; 3]));
        testCase.verifyEqual(x_y.RealBasisIndex, uint64([2; 3]));
        testCase.verifyEqual(x_y.ImaginaryBasisIndex, uint64([0; 0]));
    end
    
    function construct_matrix(testCase)
        setting = AlgebraicScenario(2);
        mat = MTKMonomial(setting, {[1], [2]; [2], [1 2]}, ...
            [1.0, 2.0; 3.0, 4.0]); %x y; x xy
        
        testCase.assertEqual(size(mat), [2 2]);
        testCase.assertEqual(numel(mat), 4);
        testCase.assertFalse(mat.IsScalar);
        testCase.assertFalse(mat.IsVector);
        testCase.assertFalse(mat.IsRowVector);
        testCase.assertFalse(mat.IsColVector);
        testCase.assertTrue(mat.IsMatrix);
        testCase.verifyEqual(mat.Operators, {uint64(1), uint64(2); ...
            uint64(2), uint64([1, 2])});
        testCase.verifyEqual(mat.Coefficient, [1, 2; 3, 4]);
        testCase.verifyEqual(mat.Hash, uint64([2, 3; 3, 5]));
        testCase.verifyEqual(mat.FoundSymbol, false(2,2));
        testCase.verifyEqual(mat.IsZero, false(2,2));
        
        setting.WordList(2, true); % Build and register symbols
        testCase.verifyEqual(mat.FoundSymbol, true(2,2));
        testCase.verifyEqual(mat.SymbolId, int64([2, 3; 3, 5]));
        testCase.verifyEqual(mat.RealBasisIndex, uint64([2, 3; 3, 5]));
        testCase.verifyEqual(mat.ImaginaryBasisIndex, uint64([0, 0;0, 1]));
    end
end

%% Find coefficients
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'coefficients'})
    function coefs_scalar(testCase)
        setting = AlgebraicScenario(2);
        xy = MTKMonomial(setting, [1 2], 1.0 + 2.0i);
        setting.WordList(2, true);
        testCase.verifyTrue(xy.FoundSymbol);
        testCase.verifyEqual(setting.System.RealVarCount, uint64(6));
        testCase.verifyEqual(setting.System.ImaginaryVarCount, uint64(1));
        
        expected_re = sparse(6, 1);
        expected_re(5) = 1 + 2.0i;
        testCase.verifyEqual(xy.RealCoefficients, expected_re);
        testCase.verifyEqual(xy.RealMask, sparse(logical([0 0 0 0 1 0])));
        testCase.verifyEqual(xy.RealBasisElements, uint64(5));
        
        expected_im = sparse(1, 1);
        expected_im(1) = (1 + 2.0i) * 1i;
        testCase.verifyEqual(xy.ImaginaryCoefficients, expected_im);
        testCase.verifyEqual(xy.ImaginaryMask, sparse(logical([1])));
        testCase.verifyEqual(xy.ImaginaryBasisElements, uint64(1));
    end

    function coefs_col_vector(testCase)
        setting = AlgebraicScenario(2);
        xy_yy_yx = MTKMonomial(setting, {[1 2]; [2 2]; [2 1]}, 1.0);
        setting.WordList(2, true);
        testCase.verifyEqual(xy_yy_yx.FoundSymbol, true(3, 1));
        testCase.verifyEqual(setting.System.RealVarCount, uint64(6));
        testCase.verifyEqual(setting.System.ImaginaryVarCount, uint64(1));
        
        expected_re = zeros(6, 3, 'like', sparse(1i));
        expected_re(5, 1) = 1;
        expected_re(6, 2) = 1;
        expected_re(5, 3) = 1;
        expected_re = [zeros(6, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(xy_yy_yx.RealCoefficients, expected_re);
        testCase.verifyEqual(xy_yy_yx.RealMask, ...
                             sparse(logical([0 0 0 0 1 1])));
        testCase.verifyEqual(xy_yy_yx.RealBasisElements, uint64([5, 6]));
        
        expected_im = zeros(1, 3, 'like', sparse(1i));
        expected_im(1,1) = 1i;
        expected_im(1,3) = -1i;
        testCase.verifyEqual(xy_yy_yx.ImaginaryCoefficients, expected_im);
        testCase.verifyEqual(xy_yy_yx.ImaginaryMask, sparse(logical([1])));        
        testCase.verifyEqual(xy_yy_yx.ImaginaryBasisElements, uint64(1));
    end

    function coefs_row_vector(testCase)
        setting = AlgebraicScenario(2);
        xy_yy_yx = MTKMonomial(setting, {[1 2], [2 2], [2 1]}, 1.0);
        setting.WordList(2, true);
        testCase.verifyEqual(xy_yy_yx.FoundSymbol, true(1, 3));
        testCase.verifyEqual(setting.System.RealVarCount, uint64(6));
        testCase.verifyEqual(setting.System.ImaginaryVarCount, uint64(1));
        
        expected_re = zeros(6, 3, 'like', sparse(1i));
        expected_re(5, 1) = 1;
        expected_re(6, 2) = 1;
        expected_re(5, 3) = 1;
        expected_re = [zeros(6, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(xy_yy_yx.RealCoefficients, expected_re);
        testCase.verifyEqual(xy_yy_yx.RealMask, ...
                             sparse(logical([0 0 0 0 1 1])));
        testCase.verifyEqual(xy_yy_yx.RealBasisElements, uint64([5, 6]));
        
        expected_im = zeros(1, 3, 'like', sparse(1i));
        expected_im(1,1) = 1i;
        expected_im(1,3) = -1i;
        testCase.verifyEqual(xy_yy_yx.ImaginaryCoefficients, expected_im);
        testCase.verifyEqual(xy_yy_yx.ImaginaryMask, sparse(logical([1])));
        testCase.verifyEqual(xy_yy_yx.ImaginaryBasisElements, uint64(1));
    end

    function coefs_matrix(testCase)
        setting = AlgebraicScenario(2);
        mat = MTKMonomial(setting, {[1], [2]; [1 2], [2 2]}, 1.0);
        setting.WordList(2, true);
        testCase.verifyEqual(mat.FoundSymbol, true(2, 2));
        testCase.verifyEqual(setting.System.RealVarCount, uint64(6));
        testCase.verifyEqual(setting.System.ImaginaryVarCount, uint64(1));
        
        expected_re = sparse(6, 4);
        expected_re(2, 1) = 1; % [1]
        expected_re(5, 2) = 1; % [1 2] (col-major!)
        expected_re(3, 3) = 1; % [2]
        expected_re(6, 4) = 1; % [2 2]
        expected_re = [zeros(6, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(mat.RealCoefficients, expected_re);
        testCase.verifyEqual(mat.RealMask, ...
                             sparse(logical([0 1 1 0 1 1])));
        testCase.verifyEqual(mat.RealBasisElements, uint64([2, 3, 5, 6]));
        
        expected_im = zeros(1, 4, 'like', sparse(1i));
        expected_im(1, 2) = 1i; % [1 2] only
        testCase.verifyEqual(mat.ImaginaryCoefficients, expected_im);
        testCase.verifyEqual(mat.ImaginaryMask, sparse(logical([1])));
        testCase.verifyEqual(mat.ImaginaryBasisElements, uint64([1]));
    end
end

%% Splice out / subsref
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'subsref'})
    
    function spliceout_no_symbols(testCase)
        setting = AlgebraicScenario(4);
        mat = MTKMonomial(setting, {[1], [2]; [3], [4]}, ...
                                    [1.0, 2.0; 3.0, 4.0]);
                                
        xy = mat(1,:);
        testCase.assertEqual(size(xy), [1 2]);
        testCase.assertTrue(isa(xy, 'MTKMonomial'));
        testCase.verifyEqual(xy.Operators, {uint64(1), uint64(2)});
        testCase.verifyEqual(xy.Coefficient, [1.0, 2.0]);
        testCase.verifyEqual(xy.Hash, uint64([2 3]));
        
        wz = mat(2,:);
        testCase.assertEqual(size(wz), [1 2]);
        testCase.assertTrue(isa(wz, 'MTKMonomial'));
        testCase.verifyEqual(wz.Operators, {uint64(3), uint64(4)});
        testCase.verifyEqual(wz.Coefficient, [3.0, 4.0]);
        testCase.verifyEqual(wz.Hash, uint64([4 5]));
        
        xw = mat(:,1);
        testCase.assertEqual(size(xw), [2 1]);
        testCase.assertTrue(isa(xw, 'MTKMonomial'));
        testCase.verifyEqual(xw.Operators, {uint64(1); uint64(3)});
        testCase.verifyEqual(xw.Coefficient, [1.0; 3.0]);
        testCase.verifyEqual(xw.Hash, uint64([2; 4]));
        
        yz = mat(:,2);
        testCase.assertEqual(size(yz), [2 1]);
        testCase.assertTrue(isa(yz, 'MTKMonomial'));
        testCase.verifyEqual(yz.Operators, {uint64(2); uint64(4)});
        testCase.verifyEqual(yz.Coefficient, [2.0; 4.0]);
        testCase.verifyEqual(yz.Hash, uint64([3; 5]));
       
        x = mat(1,1);
        testCase.assertEqual(size(x), [1 1]);
        testCase.assertTrue(isa(x, 'MTKMonomial'));
        testCase.verifyEqual(x.Operators, uint64(1));
        testCase.verifyEqual(x.Coefficient, [1.0]);
        testCase.verifyEqual(x.Hash, uint64([2]));
              
        x_alt = mat(1);
        testCase.assertEqual(size(x_alt), [1 1]);
        testCase.assertTrue(isa(x_alt, 'MTKMonomial'));
        testCase.verifyEqual(x_alt.Operators, uint64(1));
        testCase.verifyEqual(x_alt.Coefficient, [1.0]);
        testCase.verifyEqual(x_alt.Hash, uint64([2]));        
    end
    
    function spliceout_with_symbols(testCase)
        setting = AlgebraicScenario(4);
        mat = MTKMonomial(setting, {[1], [2]; [3], [4]}, ...
                                    [1.0, 2.0; 3.0, 4.0]);
        setting.WordList(1, true);
        testCase.assertEqual(mat.FoundSymbol, true(2,2));
                                
        xy = mat(1,:);
        testCase.assertEqual(size(xy), [1 2]);
        testCase.assertTrue(isa(xy, 'MTKMonomial'));
        testCase.verifyEqual(xy.Operators, {uint64(1), uint64(2)});
        testCase.verifyEqual(xy.Coefficient, [1.0, 2.0]);
        testCase.verifyEqual(xy.Hash, uint64([2 3]));
        testCase.assertEqual(xy.FoundSymbol, true(1, 2));
        testCase.verifyEqual(xy.SymbolId, int64([2, 3]));
        testCase.verifyEqual(xy.RealBasisIndex, uint64([2, 3]));
        testCase.verifyEqual(xy.ImaginaryBasisIndex, uint64([0, 0]));
        
        wz = mat(2,:);
        testCase.assertEqual(size(wz), [1 2]);
        testCase.assertTrue(isa(wz, 'MTKMonomial'));
        testCase.verifyEqual(wz.Operators, {uint64(3), uint64(4)});
        testCase.verifyEqual(wz.Coefficient, [3.0, 4.0]);
        testCase.verifyEqual(wz.Hash, uint64([4 5]));
        testCase.assertEqual(wz.FoundSymbol, true(1, 2));
        testCase.verifyEqual(wz.SymbolId, int64([4, 5]));
        testCase.verifyEqual(wz.RealBasisIndex, uint64([4, 5]));
        testCase.verifyEqual(wz.ImaginaryBasisIndex, uint64([0, 0]));
        
        xw = mat(:,1);
        testCase.assertEqual(size(xw), [2 1]);
        testCase.assertTrue(isa(xw, 'MTKMonomial'));
        testCase.verifyEqual(xw.Operators, {uint64(1); uint64(3)});
        testCase.verifyEqual(xw.Coefficient, [1.0; 3.0]);
        testCase.verifyEqual(xw.Hash, uint64([2; 4]));        
        testCase.assertEqual(xw.FoundSymbol, true(2, 1));
        testCase.verifyEqual(xw.SymbolId, int64([2; 4]));
        testCase.verifyEqual(xw.RealBasisIndex, uint64([2; 4]));
        testCase.verifyEqual(xw.ImaginaryBasisIndex, uint64([0; 0]));
        
        yz = mat(:,2);
        testCase.assertEqual(size(yz), [2 1]);
        testCase.assertTrue(isa(yz, 'MTKMonomial'));
        testCase.verifyEqual(yz.Operators, {uint64(2); uint64(4)});
        testCase.verifyEqual(yz.Coefficient, [2.0; 4.0]);
        testCase.verifyEqual(yz.Hash, uint64([3; 5]));        
        testCase.assertEqual(yz.FoundSymbol, true(2, 1));
        testCase.verifyEqual(yz.SymbolId, int64([3; 5]));
        testCase.verifyEqual(yz.RealBasisIndex, uint64([3; 5]));
        testCase.verifyEqual(yz.ImaginaryBasisIndex, uint64([0; 0]));
       
        x = mat(1,1);
        testCase.assertEqual(size(x), [1 1]);
        testCase.assertTrue(isa(x, 'MTKMonomial'));
        testCase.verifyEqual(x.Operators, uint64(1));
        testCase.verifyEqual(x.Coefficient, [1.0]);
        testCase.verifyEqual(x.Hash, uint64([2]));
        testCase.assertEqual(x.FoundSymbol, true);
        testCase.verifyEqual(x.SymbolId, int64(2));
        testCase.verifyEqual(x.RealBasisIndex, uint64(2));
        testCase.verifyEqual(x.ImaginaryBasisIndex, uint64(0));
              
        x_alt = mat(1);
        testCase.assertEqual(size(x_alt), [1 1]);
        testCase.assertTrue(isa(x_alt, 'MTKMonomial'));
        testCase.verifyEqual(x_alt.Operators, uint64(1));
        testCase.verifyEqual(x_alt.Coefficient, [1.0]);
        testCase.verifyEqual(x_alt.Hash, uint64([2]));
        testCase.assertEqual(x_alt.FoundSymbol, true);
        testCase.verifyEqual(x_alt.SymbolId, int64(2));
        testCase.verifyEqual(x_alt.RealBasisIndex, uint64(2));
        testCase.verifyEqual(x_alt.ImaginaryBasisIndex, uint64(0)); 
    end
    
    function splice_properties(testCase)
        setting = AlgebraicScenario(4);
        mat = MTKMonomial(setting, {[1], [2]; [3], [4]}, ...
                                    [1.0, 2.0; 3.0, 4.0]);
        setting.WordList(1, true);
        testCase.assertEqual(mat.FoundSymbol, true(2,2));
                                
        testCase.verifyEqual(mat(1,:).Operators, {uint64(1), uint64(2)});
        testCase.verifyEqual(mat(1,:).Coefficient, [1.0, 2.0]);
        testCase.verifyEqual(mat(1,:).Hash, uint64([2 3]));
        testCase.assertEqual(mat(1,:).FoundSymbol, true(1, 2));
        testCase.verifyEqual(mat(1,:).SymbolId, int64([2, 3]));
        testCase.verifyEqual(mat(1,:).RealBasisIndex, uint64([2, 3]));
        testCase.verifyEqual(mat(1,:).ImaginaryBasisIndex, uint64([0, 0]));

        testCase.verifyEqual(mat(2,:).Operators, {uint64(3), uint64(4)});
        testCase.verifyEqual(mat(2,:).Coefficient, [3.0, 4.0]);
        testCase.verifyEqual(mat(2,:).Hash, uint64([4 5]));
        testCase.assertEqual(mat(2,:).FoundSymbol, true(1, 2));
        testCase.verifyEqual(mat(2,:).SymbolId, int64([4, 5]));
        testCase.verifyEqual(mat(2,:).RealBasisIndex, uint64([4, 5]));
        testCase.verifyEqual(mat(2,:).ImaginaryBasisIndex, uint64([0, 0]));
        
        testCase.verifyEqual(mat(:,1).Operators, {uint64(1); uint64(3)});
        testCase.verifyEqual(mat(:,1).Coefficient, [1.0; 3.0]);
        testCase.verifyEqual(mat(:,1).Hash, uint64([2; 4]));        
        testCase.assertEqual(mat(:,1).FoundSymbol, true(2, 1));
        testCase.verifyEqual(mat(:,1).SymbolId, int64([2; 4]));
        testCase.verifyEqual(mat(:,1).RealBasisIndex, uint64([2; 4]));
        testCase.verifyEqual(mat(:,1).ImaginaryBasisIndex, uint64([0; 0]));
        
        testCase.verifyEqual(mat(:,2).Operators, {uint64(2); uint64(4)});
        testCase.verifyEqual(mat(:,2).Coefficient, [2.0; 4.0]);
        testCase.verifyEqual(mat(:,2).Hash, uint64([3; 5]));        
        testCase.assertEqual(mat(:,2).FoundSymbol, true(2, 1));
        testCase.verifyEqual(mat(:,2).SymbolId, int64([3; 5]));
        testCase.verifyEqual(mat(:,2).RealBasisIndex, uint64([3; 5]));
        testCase.verifyEqual(mat(:,2).ImaginaryBasisIndex, uint64([0; 0]));

        testCase.verifyEqual(mat(1,1).Operators, uint64(1));
        testCase.verifyEqual(mat(1,1).Coefficient, [1.0]);
        testCase.verifyEqual(mat(1,1).Hash, uint64([2]));
        testCase.assertEqual(mat(1,1).FoundSymbol, true);
        testCase.verifyEqual(mat(1,1).SymbolId, int64(2));
        testCase.verifyEqual(mat(1,1).RealBasisIndex, uint64(2));
        testCase.verifyEqual(mat(1,1).ImaginaryBasisIndex, uint64(0));

        testCase.verifyEqual(mat(1).Operators, uint64(1));
        testCase.verifyEqual(mat(1).Coefficient, [1.0]);
        testCase.verifyEqual(mat(1).Hash, uint64([2]));
        testCase.assertEqual(mat(1).FoundSymbol, true);
        testCase.verifyEqual(mat(1).SymbolId, int64(2));
        testCase.verifyEqual(mat(1).RealBasisIndex, uint64(2));
        testCase.verifyEqual(mat(1).ImaginaryBasisIndex, uint64(0)); 
    end
end

%% Concatenation
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'cat'})
     function cat_vert_scalars(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();
        setting.WordList(1, true);
        
        two_y = 2*y;        
        testCase.assertNotEmpty(x.RealCoefficients);
        testCase.assertNotEmpty(two_y.RealCoefficients);
        testCase.assertNotEmpty(z.RealCoefficients);
        
        xyz = [x; two_y; z];
        testCase.assertTrue(isa(xyz, 'MTKMonomial'));
        testCase.assertEqual(size(xyz), [3 1]);
        testCase.assertFalse(xyz.IsScalar);
        testCase.assertTrue(xyz.IsVector);
        testCase.assertTrue(xyz.IsColVector);
        testCase.verifyEqual(xyz.Operators, {uint64(1); uint64(2); uint64(3)});
        testCase.verifyEqual(xyz.Coefficient, [1; 2; 1]);
        testCase.verifyEqual(xyz.Hash, uint64([2; 3; 4]));
        testCase.verifyEqual(xyz.FoundSymbol, true(3, 1));
        testCase.verifyEqual(xyz.SymbolId, int64([2; 3; 4]));
        testCase.verifyEqual(xyz.RealBasisIndex, uint64([2; 3; 4]));
        testCase.verifyEqual(xyz.ImaginaryBasisIndex, uint64([0; 0; 0]));   
        
        expected_re = sparse(4, 3);
        expected_re(2, 1) = 1;
        expected_re(3, 2) = 2;
        expected_re(4, 3) = 1;
        expected_re = [zeros(4, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(xyz.RealCoefficients, expected_re);
        
        expected_im = zeros(0, 3, 'like', sparse(1i));
        testCase.verifyEqual(xyz.ImaginaryCoefficients, expected_im);
     end
    
     function cat_vert_row_to_matrix(testCase)
        setting = AlgebraicScenario(4);
        
        w_x = MTKMonomial(setting, {[1], [2]}, 1.0);
        y_z = MTKMonomial(setting, {[3], [4]}, -1.0);
        setting.WordList(1, true);
        
        testCase.assertNotEmpty(w_x.RealCoefficients);
        testCase.assertNotEmpty(y_z.RealCoefficients);
                
        mat = [w_x; y_z];
        testCase.assertTrue(isa(mat, 'MTKMonomial'));
        testCase.assertEqual(size(mat), [2 2]);        
        testCase.assertFalse(mat.IsScalar);
        testCase.assertFalse(mat.IsVector);
        testCase.assertTrue(mat.IsMatrix);
        testCase.verifyEqual(mat.Operators, ...
            {uint64(1), uint64(2); uint64(3), uint64(4)});
        testCase.verifyEqual(mat.Coefficient, [1, 1; -1, -1]);
        testCase.verifyEqual(mat.Hash, uint64([2, 3; 4, 5]));
        testCase.verifyEqual(mat.FoundSymbol, true(2, 2));
        testCase.verifyEqual(mat.SymbolId, int64([2 3; 4 5]));
        testCase.verifyEqual(mat.RealBasisIndex, uint64([2 3; 4 5]));
        testCase.verifyEqual(mat.ImaginaryBasisIndex, uint64([0 0; 0 0]));
        
        expected_re = sparse(5, 4);
        expected_re(2, 1) = 1;
        expected_re(4, 2) = -1;
        expected_re(3, 3) = 1;        
        expected_re(5, 4) = -1;
        expected_re = [zeros(5, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(mat.RealCoefficients, expected_re);
        
        expected_im = zeros(0, 4, 'like', sparse(1i));
        testCase.verifyEqual(mat.ImaginaryCoefficients, expected_im);
     end
    
     function cat_vert_matrix_to_matrix(testCase)
        setting = AlgebraicScenario(4, 'hermitian', false);
        setting.WordList(1, true);
        testCase.assertEqual(setting.RawOperatorCount, uint64(8));
        
        matA = MTKMonomial(setting, {[1], [3]; [5], [7]}, 1.0);
        matB = MTKMonomial(setting, {[2], [4]; [6], [8]}, -1.0);
        
        testCase.assertNotEmpty(matA.RealCoefficients);
        testCase.assertNotEmpty(matB.RealCoefficients);
               
        mat = [matA; matB];
        
        testCase.assertTrue(isa(mat, 'MTKMonomial'));
        testCase.assertEqual(size(mat), [4 2]);        
        testCase.assertTrue(mat.IsMatrix);
        testCase.verifyEqual(mat.Operators, ...
            {uint64(1), uint64(3); 
             uint64(5), uint64(7);
             uint64(2), uint64(4);
             uint64(6), uint64(8)});
        testCase.verifyEqual(mat.Coefficient, [1, 1; 1, 1; -1, -1; -1, -1]);
        testCase.verifyEqual(mat.Hash, uint64([2 4; 6 8; 3 5; 7 9]));
        testCase.verifyEqual(mat.FoundSymbol, true(4, 2));
        
        testCase.verifyEqual(mat.SymbolId, int64([2 3; 4 5; 2 3; 4 5]));
        testCase.verifyEqual(mat.SymbolConjugated, ...
                             logical([0 0; 0 0; 1 1; 1 1]));
        testCase.verifyEqual(mat.RealBasisIndex, ...
                             uint64([2 3; 4 5; 2 3; 4 5]));
        testCase.verifyEqual(mat.ImaginaryBasisIndex, ...
                             uint64([1 2; 3 4; 1 2; 3 4]));
        
        expected_re = sparse(5, 8);
        expected_re(2, 1) = 1;
        expected_re(4, 2) = 1;
        expected_re(2, 3) = -1;        
        expected_re(4, 4) = -1;        
        expected_re(3, 5) = 1;
        expected_re(5, 6) = 1;
        expected_re(3, 7) = -1;        
        expected_re(5, 8) = -1;
        expected_re = [zeros(5, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(mat.RealCoefficients, expected_re);
        
        expected_im = sparse(4, 8);
        expected_im(1, 1) = 1i;
        expected_im(3, 2) = 1i;
        expected_im(1, 3) = 1i;
        expected_im(3, 4) = 1i;
        expected_im(2, 5) = 1i;
        expected_im(4, 6) = 1i;
        expected_im(2, 7) = 1i;
        expected_im(4, 8) = 1i;        
        testCase.verifyEqual(mat.ImaginaryCoefficients, expected_im);
     end
     
     function cat_horz_scalars(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();
        setting.WordList(1, true);
        
        testCase.assertNotEmpty(x.RealCoefficients);
        testCase.assertNotEmpty(y.RealCoefficients);
        testCase.assertNotEmpty(z.RealCoefficients);
                
        xyz = [x, y, z];
        testCase.assertTrue(isa(xyz, 'MTKMonomial'));
        testCase.assertEqual(size(xyz), [1 3]);        
        testCase.assertFalse(xyz.IsScalar);
        testCase.assertTrue(xyz.IsVector);
        testCase.assertTrue(xyz.IsRowVector);
        testCase.verifyEqual(xyz.Operators, {uint64(1), uint64(2), uint64(3)});
        testCase.verifyEqual(xyz.Coefficient, [1, 1, 1]);
        testCase.verifyEqual(xyz.Hash, uint64([2, 3, 4]));
        testCase.verifyEqual(xyz.FoundSymbol, true(1, 3));
        testCase.verifyEqual(xyz.SymbolId, int64([2 3 4]));
        testCase.verifyEqual(xyz.RealBasisIndex, uint64([2 3 4]));
        testCase.verifyEqual(xyz.ImaginaryBasisIndex, uint64([0 0 0]));
        
        expected_re = sparse(4, 3);
        expected_re(2, 1) = 1;
        expected_re(3, 2) = 1;
        expected_re(4, 3) = 1;
        expected_re = [zeros(4, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(xyz.RealCoefficients, expected_re);
        
        expected_im = zeros(0, 3, 'like', sparse(1i));
        testCase.verifyEqual(xyz.ImaginaryCoefficients, expected_im);
     end
     
     function cat_horz_row_vectors(testCase)
        setting = AlgebraicScenario(4);
        
        w_x = MTKMonomial(setting, {[1], [2]}, 1.0);
        y_z = MTKMonomial(setting, {[3], [4]}, -1.0);
        setting.WordList(1, true);
        
        testCase.assertNotEmpty(w_x.RealCoefficients);
        testCase.assertNotEmpty(y_z.RealCoefficients);
                
        wxyz = [w_x, y_z];
        testCase.assertTrue(isa(wxyz, 'MTKMonomial'));
        testCase.assertEqual(size(wxyz), [1 4]);        
        testCase.assertFalse(wxyz.IsScalar);
        testCase.assertTrue(wxyz.IsVector);
        testCase.assertTrue(wxyz.IsRowVector);
        testCase.verifyEqual(wxyz.Operators, ...
            {uint64(1), uint64(2), uint64(3), uint64(4)});
        testCase.verifyEqual(wxyz.Coefficient, [1, 1, -1, -1]);
        testCase.verifyEqual(wxyz.Hash, uint64([2, 3, 4, 5]));
        testCase.verifyEqual(wxyz.FoundSymbol, true(1, 4));
        testCase.verifyEqual(wxyz.SymbolId, int64([2 3 4 5]));
        testCase.verifyEqual(wxyz.RealBasisIndex, uint64([2 3 4 5]));
        testCase.verifyEqual(wxyz.ImaginaryBasisIndex, uint64([0 0 0 0]));
        
        expected_re = sparse(5, 4);
        expected_re(2, 1) = 1;
        expected_re(3, 2) = 1;
        expected_re(4, 3) = -1;
        expected_re(5, 4) = -1;
        expected_re = [zeros(5, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(wxyz.RealCoefficients, expected_re);
        
        expected_im = zeros(0, 4, 'like', sparse(1i));
        testCase.verifyEqual(wxyz.ImaginaryCoefficients, expected_im);
     end
     
       
     function cat_horz_mixed_row(testCase)
        setting = AlgebraicScenario(4);
        
        x = MTKMonomial(setting, 2, 1i);
        w_x = MTKMonomial(setting, {[1], [2]}, 1.0);
        y_z = MTKMonomial(setting, {[3], [4]}, -1.0);
        setting.WordList(1, true);
        
        testCase.assertNotEmpty(x.RealCoefficients);
        testCase.assertNotEmpty(w_x.RealCoefficients);
        testCase.assertNotEmpty(y_z.RealCoefficients);
                
        wxxyz = [w_x, x, y_z];
        testCase.assertTrue(isa(wxxyz, 'MTKMonomial'));
        testCase.assertEqual(size(wxxyz), [1 5]);        
        testCase.assertFalse(wxxyz.IsScalar);
        testCase.assertTrue(wxxyz.IsVector);
        testCase.assertTrue(wxxyz.IsRowVector);
        testCase.verifyEqual(wxxyz.Operators, ...
            {uint64(1), uint64(2), uint64(2), uint64(3), uint64(4)});
        testCase.verifyEqual(wxxyz.Coefficient, [1, 1, 1i, -1, -1]);
        testCase.verifyEqual(wxxyz.Hash, uint64([2, 3, 3, 4, 5]));
        testCase.verifyEqual(wxxyz.FoundSymbol, true(1, 5));
        testCase.verifyEqual(wxxyz.SymbolId, int64([2 3 3 4 5]));
        testCase.verifyEqual(wxxyz.RealBasisIndex, uint64([2 3 3 4 5]));
        testCase.verifyEqual(wxxyz.ImaginaryBasisIndex, uint64([0 0 0 0 0]));
        
        expected_re = sparse(5, 4);
        expected_re(2, 1) = 1;
        expected_re(3, 2) = 1;
        expected_re(3, 3) = 1i;
        expected_re(4, 4) = -1;
        expected_re(5, 5) = -1;
        expected_re = [zeros(5, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(wxxyz.RealCoefficients, expected_re);
        
        expected_im = zeros(0, 5, 'like', sparse(1i));
        testCase.verifyEqual(wxxyz.ImaginaryCoefficients, expected_im);
     end
     
      function cat_horz_col_to_matrix(testCase)
        setting = AlgebraicScenario(4);
        
        w_x = MTKMonomial(setting, {[1]; [2]}, 1.0);
        y_z = MTKMonomial(setting, {[3]; [4]}, -1.0);
        setting.WordList(1, true);
        
        testCase.assertNotEmpty(w_x.RealCoefficients);
        testCase.assertNotEmpty(y_z.RealCoefficients);
                
        mat = [w_x, y_z];
        testCase.assertTrue(isa(mat, 'MTKMonomial'));
        testCase.assertEqual(size(mat), [2 2]);        
        testCase.assertFalse(mat.IsScalar);
        testCase.assertFalse(mat.IsVector);
        testCase.assertTrue(mat.IsMatrix);
        testCase.verifyEqual(mat.Operators, ...
            {uint64(1), uint64(3); uint64(2), uint64(4)});
        testCase.verifyEqual(mat.Coefficient, [1, -1; 1, -1]);
        testCase.verifyEqual(mat.Hash, uint64([2, 4; 3, 5]));
        testCase.verifyEqual(mat.FoundSymbol, true(2, 2));
        testCase.verifyEqual(mat.SymbolId, int64([2 4; 3 5]));
        testCase.verifyEqual(mat.RealBasisIndex, uint64([2 4; 3 5]));
        testCase.verifyEqual(mat.ImaginaryBasisIndex, uint64([0 0; 0 0]));
        
        expected_re = sparse(5, 4);
        expected_re(2, 1) = 1;
        expected_re(3, 2) = 1;
        expected_re(4, 3) = -1;
        expected_re(5, 4) = -1;
        expected_re = [zeros(5, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(mat.RealCoefficients, expected_re);
        
        expected_im = zeros(0, 4, 'like', sparse(1i));
        testCase.verifyEqual(mat.ImaginaryCoefficients, expected_im);
      end
    
   function cat_horz_matrix_to_matrix(testCase)
        setting = AlgebraicScenario(4, 'hermitian', false);
        setting.WordList(1, true);
        testCase.assertEqual(setting.RawOperatorCount, uint64(8));
        
        matA = MTKMonomial(setting, {[1], [3]; [5], [7]}, 1.0);
        matB = MTKMonomial(setting, {[2], [4]; [6], [8]}, -1.0);
        
        testCase.assertNotEmpty(matA.RealCoefficients);
        testCase.assertNotEmpty(matB.RealCoefficients);
               
        mat = [matA, matB];
        
        testCase.assertTrue(isa(mat, 'MTKMonomial'));
        testCase.assertEqual(size(mat), [2 4]);        
        testCase.assertTrue(mat.IsMatrix);
        testCase.verifyEqual(mat.Operators, ...
            {uint64(1), uint64(3), uint64(2), uint64(4);
             uint64(5), uint64(7), uint64(6), uint64(8)});
        testCase.verifyEqual(mat.Coefficient, [1, 1, -1, -1; 1, 1, -1, -1]);
        testCase.verifyEqual(mat.Hash, uint64([2 4 3 5; 6 8 7 9]));
        testCase.verifyEqual(mat.FoundSymbol, true(2, 4));
        
        testCase.verifyEqual(mat.SymbolId, int64([2 3 2 3; 4 5 4 5]));
        testCase.verifyEqual(mat.SymbolConjugated, ...
                             logical([0 0 1 1; 0 0 1 1]));
        testCase.verifyEqual(mat.RealBasisIndex, ...
                             uint64([2 3 2 3; 4 5 4 5]));
        testCase.verifyEqual(mat.ImaginaryBasisIndex, ...
                             uint64([1 2 1 2; 3 4 3 4]));
        
        expected_re = sparse(5, 8);
        expected_re(2, 1) = 1;
        expected_re(4, 2) = 1;
        expected_re(3, 3) = 1;        
        expected_re(5, 4) = 1;        
        expected_re(2, 5) = -1;
        expected_re(4, 6) = -1;
        expected_re(3, 7) = -1;        
        expected_re(5, 8) = -1;
        expected_re = [zeros(5, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(mat.RealCoefficients, expected_re);
        
        expected_im = sparse(4, 8);
        expected_im(1, 1) = 1i;
        expected_im(3, 2) = 1i;
        expected_im(2, 3) = 1i;
        expected_im(4, 4) = 1i;
        expected_im(1, 5) = 1i;
        expected_im(3, 6) = 1i;
        expected_im(2, 7) = 1i;
        expected_im(4, 8) = 1i;        
        testCase.verifyEqual(mat.ImaginaryCoefficients, expected_im);
   end
     
   function cat_3_matrix_to_tensor(testCase)
        setting = AlgebraicScenario(4, 'hermitian', false);
        setting.WordList(1, true);
        testCase.assertEqual(setting.RawOperatorCount, uint64(8));
        
        matA = MTKMonomial(setting, {[1], [3]; [5], [7]}, 1.0);
        matB = MTKMonomial(setting, {[2], [4]; [6], [8]}, -1.0);
        
        testCase.assertNotEmpty(matA.RealCoefficients);
        testCase.assertNotEmpty(matB.RealCoefficients);
               
        tensor = cat(3, matA, matB);
        
        testCase.assertTrue(isa(tensor, 'MTKMonomial'));
        testCase.assertEqual(size(tensor), [2 2 2]);        
        testCase.assertFalse(tensor.IsMatrix);
        testCase.verifyEqual(tensor.Operators, ...
            cat(3, {uint64(1), uint64(3); uint64(5), uint64(7)}, ...
                   {uint64(2), uint64(4); uint64(6), uint64(8)}));
               
        testCase.verifyEqual(tensor.Coefficient, ...
            cat(3, [1 1; 1 1], [-1 -1; -1 -1]));
        
        testCase.verifyEqual(tensor.Hash, ...
                           cat(3, uint64([2 4; 6 8]), uint64([3 5; 7 9])));
        testCase.verifyEqual(tensor.FoundSymbol, true(2, 2, 2));
        
        testCase.verifyEqual(tensor.SymbolId, ...
                          cat(3, int64([2 3; 4 5]), int64([2 3; 4 5])));
        testCase.verifyEqual(tensor.SymbolConjugated, ...
                          cat(3, logical([0 0; 0 0]), logical([1 1; 1 1])));
        testCase.verifyEqual(tensor.RealBasisIndex, ...
                          cat(3, uint64([2 3; 4 5]), uint64([2 3; 4 5])));
        testCase.verifyEqual(tensor.ImaginaryBasisIndex, ...
                          cat(3, uint64([1 2; 3 4]), uint64([1 2; 3 4])));
        
        expected_re = sparse(5, 8);
        expected_re(2, 1) = 1;
        expected_re(4, 2) = 1;
        expected_re(3, 3) = 1;        
        expected_re(5, 4) = 1;        
        expected_re(2, 5) = -1;
        expected_re(4, 6) = -1;
        expected_re(3, 7) = -1;        
        expected_re(5, 8) = -1;
        expected_re = [zeros(5, 0, 'like', sparse(1i)), expected_re];
        testCase.verifyEqual(tensor.RealCoefficients, expected_re);
        
        expected_im = sparse(4, 8);
        expected_im(1, 1) = 1i;
        expected_im(3, 2) = 1i;
        expected_im(2, 3) = 1i;
        expected_im(4, 4) = 1i;
        expected_im(1, 5) = 1i;
        expected_im(3, 6) = 1i;
        expected_im(2, 7) = 1i;
        expected_im(4, 8) = 1i;        
        testCase.verifyEqual(tensor.ImaginaryCoefficients, expected_im);
     end
end

%% Symbol cells
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'SymbolCell'})
    function symbol_cell_x(testCase)
        setting = AlgebraicScenario(2, 'hermitian', false);
        setting.WordList(1, true);
        x_conj = MTKMonomial(setting, [2], -2.0);
        
        testCase.assertTrue(x_conj.FoundSymbol);
        testCase.verifyEqual(x_conj.SymbolCell, ...
            {{{int64(2), -2.0, true}}});        
    end
         
    function symbol_cell_row_vector(testCase)
        setting = AlgebraicScenario(2, 'hermitian', false);
        setting.WordList(1, true);
        x_y_conj = MTKMonomial(setting, {[1], [4]}, 10.0);

        testCase.assertEqual(x_y_conj.FoundSymbol, [true true]);        
        testCase.verifyEqual(x_y_conj.SymbolCell, ...
            {{{int64(2), 10.0}}, {{int64(3), 10.0, true}}});
    end

    function symbol_cell_matrix(testCase)
        setting = AlgebraicScenario(2, 'hermitian', false);
        setting.WordList(1, true);
        mat = MTKMonomial(setting, {[1], [4]; [3], [2]}, 2i);

        testCase.assertEqual(mat.FoundSymbol, true(2,2));
        testCase.verifyEqual(mat.SymbolCell, ...
            {{{int64(2), 2i}}, {{int64(3), 2i, true}};
             {{int64(3), 2i}}, {{int64(2), 2i, true}}});
    end
    
    function symbol_cell_cat(testCase)
        setting = AlgebraicScenario(2, 'hermitian', false);
        setting.WordList(1, true);
        vecA = MTKMonomial(setting, {[1]; [3]}, 2i); 
        vecB = MTKMonomial(setting, {[4]; [2]}, 2i);
        testCase.assertNotEmpty(vecA.SymbolCell);
        testCase.assertNotEmpty(vecB.SymbolCell);
        mat = [vecA, vecB];

        testCase.assertEqual(mat.FoundSymbol, true(2,2));
        testCase.verifyEqual(mat.SymbolCell, ...
            {{{int64(2), 2i}}, {{int64(3), 2i, true}};
             {{int64(3), 2i}}, {{int64(2), 2i, true}}});
    end
end

%% Equality (eq)
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'eq'})
    function eq_mono_mono(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();
        testCase.verifyFalse(x == y);
        testCase.verifyTrue(x == x);
        testCase.verifyTrue(y == y);
        x2 = setting.get([1]);
        testCase.verifyTrue(x == x2);
    end

    function eq_mono_mono_two_settings(testCase)
        setting = AlgebraicScenario(2);
        other_setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();
        [x2, y2] = other_setting.getAll();
        testCase.verifyFalse(x == x2);
        testCase.verifyFalse(x == y2);
        testCase.verifyFalse(y == x2);
        testCase.verifyFalse(y == y2);
    end

    function eq_mono_double(testCase)
        setting = AlgebraicScenario(2);
        two = MTKMonomial(setting, [], 2.0);
        testCase.verifyFalse(two == 1.0);
        testCase.verifyFalse(1.0 == two);
        testCase.verifyTrue(two == 2.0);
        testCase.verifyTrue(2.0 == two);
    end
end

%% Unary plus (uplus)
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'algebraic', 'uplus'})
    function uplus_zero(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero;
        plus_zero = +zero;

        testCase.verifyTrue(isa(plus_zero, 'MTKMonomial'));
        testCase.assertTrue(plus_zero.IsZero);
        testCase.verifyEqual(plus_zero.Scenario, setting);
    end

    function uplus_mono(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        i_x = +x;

        testCase.verifyTrue(isa(i_x, 'MTKMonomial'));
        testCase.assertEqual(i_x.Operators, uint64([1]));
        testCase.assertEqual(i_x.Coefficient, 1.0);
    end

    function uplus_vector(testCase)
        setting = AlgebraicScenario(2);
        wl = setting.WordList(1);
        plus_wl = +wl;

        testCase.verifyTrue(isa(plus_wl, 'MTKMonomial'));
        testCase.verifyTrue(all(wl == plus_wl));
    end
end

%% Unary minus (uminus)
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'algebraic', 'uminus'})
    function uminus_zero(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero;
        minus_zero = -zero;

        testCase.verifyTrue(isa(minus_zero, 'MTKMonomial'));
        testCase.assertTrue(minus_zero.IsZero);
        testCase.verifyEqual(minus_zero.Scenario, setting);
    end

    function uminus_mono(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        i_x = -x;

        testCase.verifyTrue(isa(i_x, 'MTKMonomial'));
        testCase.assertEqual(i_x.Operators, uint64([1]));
        testCase.assertEqual(i_x.Coefficient, -1.0);
    end

    function uminus_vector(testCase)
        setting = AlgebraicScenario(2);
        wl = setting.WordList(1);
        minus_wl = -wl;

        testCase.verifyTrue(isa(minus_wl, 'MTKMonomial'));
        testCase.verifyTrue(all(minus_wl.Coefficient == -1));
        testCase.verifyTrue(all(isequal(wl.Operators, minus_wl.Operators)))
    end
end

%% Addition (plus)
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'algebraic', 'plus'})
    function plus_zero_num(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero();

        also_zero = zero + 0.0;
        testCase.assertTrue(isa(also_zero, 'MTKMonomial'));
        testCase.verifyTrue(also_zero.IsZero);
        testCase.assertEqual(also_zero.Scenario, setting);

        not_zero = zero + 5.0;
        testCase.assertTrue(isa(not_zero, 'MTKMonomial'));
        testCase.assertEqual(not_zero.Scenario, setting);
        testCase.verifyEqual(not_zero.Operators, uint64.empty(1,0));
        testCase.verifyEqual(not_zero.Coefficient, 5.0);

    end

    function plus_num_zero(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero();

        also_zero = 0.0 + zero;
        testCase.verifyTrue(also_zero.IsZero);
        testCase.assertEqual(also_zero.Scenario, setting);

        not_zero = 5.0 + zero;
        testCase.verifyTrue(isa(not_zero, 'MTKMonomial'));
        testCase.assertEqual(not_zero.Scenario, setting);
        testCase.verifyEqual(not_zero.Operators, uint64.empty(1,0));
        testCase.verifyEqual(not_zero.Coefficient, 5.0);
    end

    function plus_num_mono(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        i_x = 1 + x;

        testCase.verifyTrue(isa(i_x, 'MTKPolynomial'));
        testCase.assertEqual(length(i_x.Constituents), 2);
        part_i = i_x.Constituents(1);
        part_x = i_x.Constituents(2);
        testCase.assertEqual(part_i.Operators, uint64.empty(1,0));
        testCase.assertEqual(part_i.Coefficient, 1.0);
        testCase.assertEqual(part_x.Operators, uint64([1]));
        testCase.assertEqual(part_x.Coefficient, 1.0);

    end

    function plus_mono_num(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        testCase.verifyEqual(x.Operators, uint64([1]));

        x_i = x + 1;

        testCase.verifyTrue(isa(x_i, 'MTKPolynomial'));
        testCase.assertEqual(length(x_i.Constituents), 2);
        part_i = x_i.Constituents(1);
        part_x = x_i.Constituents(2);
        testCase.assertEqual(part_i.Operators, uint64.empty(1,0));
        testCase.assertEqual(part_i.Coefficient, 1.0);
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
    end


    function plus_zero_mono(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero();
        [x, ~] = setting.getAll();

        also_x = zero + x;
        testCase.verifyTrue(eq(also_x, x));
    end

    function plus_mono_zero(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero();
        [x, ~] = setting.getAll();

        also_x = x + zero;
        testCase.verifyTrue(eq(also_x, x));
    end


    function plus_mono_mono(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_plus_y = x + y;
        y_plus_x = y + x;
        testCase.verifyTrue(isa(x_plus_y, 'MTKPolynomial'));
        testCase.assertEqual(length(x_plus_y.Constituents), 2);
        part_x = x_plus_y.Constituents(1);
        part_y = x_plus_y.Constituents(2);
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
        testCase.assertEqual(part_y.Operators, y.Operators);
        testCase.assertEqual(part_y.Coefficient, y.Coefficient);

        testCase.verifyTrue(isa(y_plus_x, 'MTKPolynomial'));
        testCase.assertEqual(length(y_plus_x.Constituents), 2);
        partB_x = y_plus_x.Constituents(1);
        partB_y = y_plus_x.Constituents(2);
        testCase.assertEqual(partB_x.Operators, x.Operators);
        testCase.assertEqual(partB_x.Coefficient, x.Coefficient);
        testCase.assertEqual(partB_y.Operators, y.Operators);
        testCase.assertEqual(partB_y.Coefficient, y.Coefficient);
    end

    function plus_mono_mono_same(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);

        x_plus_x = x + x;
        testCase.verifyTrue(isa(x_plus_x, 'MTKMonomial'));
        testCase.verifyEqual(x_plus_x.Operators, x.Operators);
        testCase.verifyEqual(x_plus_x.Coefficient, 2 * x.Coefficient);
    end


    function plus_mono_mono_different_length(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get([1]);
        xy = setting.get([1, 2]);

        x_plus_xy = x + xy;
        xy_plus_x = xy + x;
        testCase.assertTrue(isa(x_plus_xy, 'MTKPolynomial'));
        testCase.assertEqual(length(x_plus_xy.Constituents), 2);
        part_x = x_plus_xy.Constituents(1);
        part_xy = x_plus_xy.Constituents(2);
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
        testCase.assertEqual(part_xy.Operators, xy.Operators);
        testCase.assertEqual(part_xy.Coefficient, xy.Coefficient);

        testCase.verifyTrue(isa(xy_plus_x, 'MTKPolynomial'));
        testCase.assertEqual(length(xy_plus_x.Constituents), 2);
        partB_x = xy_plus_x.Constituents(1);
        partB_xy = xy_plus_x.Constituents(2);
        testCase.assertEqual(partB_x.Operators, x.Operators);
        testCase.assertEqual(partB_x.Coefficient, x.Coefficient);
        testCase.assertEqual(partB_xy.Operators, xy.Operators);
        testCase.assertEqual(partB_xy.Coefficient, xy.Coefficient);
    end

    function plus_vector_number(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll;

        wl = setting.WordList(1);
        wl_plus_1 = wl + 1.0;
        testCase.assertTrue(isa(wl_plus_1, 'MTKPolynomial'));
        testCase.assertEqual(size(wl_plus_1), [3 1]);
        testCase.verifyTrue(wl_plus_1(1) == 2.0);
        testCase.verifyTrue(wl_plus_1(2) == x + 1.0);
        testCase.verifyTrue(wl_plus_1(3) == y + 1.0);

        one_plus_wl = 1.0 + wl;
        testCase.verifyTrue(isequal(one_plus_wl, wl_plus_1));
    end

    function plus_broadcast_number(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        rhs = [1, 2; 3, 4];
        xp = x + rhs;
        testCase.assertTrue(isa(xp, 'MTKPolynomial'));
        testCase.assertEqual(size(xp), [2 2]);
        testCase.verifyTrue(xp(1,1) == x + 1.0);
        testCase.verifyTrue(xp(2,1) == x + 3.0);
        testCase.verifyTrue(xp(1,2) == x + 2.0);
        testCase.verifyTrue(xp(2,2) == x + 4.0);

        xp_prime = rhs + x;
        testCase.verifyTrue(isequal(xp, xp_prime));
    end
end

%% Subtraction (minus)
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'algebraic', 'minus'})
    function minus_num_mono(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        i_x = 1 - x;

        testCase.verifyTrue(isa(i_x, 'MTKPolynomial'));
        testCase.assertEqual(length(i_x.Constituents), 2);
        part_i = i_x.Constituents(1);
        part_x = i_x.Constituents(2);
        testCase.assertEqual(part_i.Operators, uint64.empty(1,0));
        testCase.assertEqual(part_i.Coefficient, 1.0);
        testCase.assertEqual(part_x.Operators, uint64([1]));
        testCase.assertEqual(part_x.Coefficient, -1.0);

    end

    function minus_mono_num(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        testCase.verifyEqual(x.Operators, uint64([1]));

        x_i = x - 1;

        testCase.verifyTrue(isa(x_i, 'MTKPolynomial'));
        testCase.assertEqual(length(x_i.Constituents), 2);
        part_i = x_i.Constituents(1);
        part_x = x_i.Constituents(2);
        testCase.assertEqual(part_i.Operators, uint64.empty(1,0));
        testCase.assertEqual(part_i.Coefficient, -1.0);
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
    end

    function minus_mono_mono(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_minus_y = x - y;
        y_minus_x = y - x;
        testCase.verifyTrue(isa(x_minus_y, 'MTKPolynomial'));
        testCase.assertEqual(length(x_minus_y.Constituents), 2);
        part_x = x_minus_y.Constituents(1);
        part_y = x_minus_y.Constituents(2);
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
        testCase.assertEqual(part_y.Operators, y.Operators);
        testCase.assertEqual(part_y.Coefficient, -y.Coefficient);

        testCase.verifyTrue(isa(y_minus_x, 'MTKPolynomial'));
        testCase.assertEqual(length(y_minus_x.Constituents), 2);
        partB_x = y_minus_x.Constituents(1);
        partB_y = y_minus_x.Constituents(2);
        testCase.assertEqual(partB_x.Operators, x.Operators);
        testCase.assertEqual(partB_x.Coefficient, -x.Coefficient);
        testCase.assertEqual(partB_y.Operators, y.Operators);
        testCase.assertEqual(partB_y.Coefficient, y.Coefficient);
    end

    function minus_mono_mono_same(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);

        x_minus_x = x - x;
        testCase.verifyTrue(x_minus_x.IsZero);
        testCase.verifyEqual(x_minus_x.Scenario, setting);
    end

    function minus_vector(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();
        wl = setting.WordList(1);
        wl_minus_1 = wl - 1.0;
        testCase.assertTrue(isa(wl_minus_1, 'MTKPolynomial'));
        testCase.assertEqual(size(wl_minus_1), [4, 1]);
        testCase.verifyTrue(wl_minus_1(1) == 0);
        testCase.verifyTrue(wl_minus_1(2) == x-1.0);
        testCase.verifyTrue(wl_minus_1(3) == y-1.0);
        testCase.verifyTrue(wl_minus_1(4) == z-1.0);
    end

    function minus_broadcast(testCase)
        setting = AlgebraicScenario(3);
        x = setting.get(1);
        nums = [1, 2; 3, 4];

        x_minus_nums = x - nums;

        testCase.assertTrue(isa(x_minus_nums, 'MTKPolynomial'));
        testCase.assertEqual(size(x_minus_nums), [2, 2]);
        testCase.verifyTrue(x_minus_nums(1, 1) == x-1.0);
        testCase.verifyTrue(x_minus_nums(1, 2) == x-2.0);
        testCase.verifyTrue(x_minus_nums(2, 1) == x-3.0);
        testCase.verifyTrue(x_minus_nums(2, 2) == x-4.0);
    end
end

%% Elementwise multiplication (times)
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'algebraic', 'times'})
    function times_zero_mono(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get([1]);
        zero = setting.zero();
        also_zero = zero .* x;
        testCase.assertTrue(also_zero.IsZero);
        testCase.verifyEqual(also_zero.Scenario, setting);
    end
    
    function times_scalar_empty(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get([1]);
        x_zero = 0 * x;
        testCase.assertTrue(x_zero.IsZero);
        testCase.verifyEqual(x_zero.Scenario, setting);
        also_zero = 2 * x_zero;
        testCase.assertTrue(also_zero.IsZero);
        testCase.verifyEqual(also_zero.Scenario, setting);
    end

    function times_mono_zero(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get([1]);
        zero = setting.zero();
        also_zero = x .* zero;
        testCase.assertTrue(also_zero.IsZero);
        testCase.verifyEqual(also_zero.Scenario, setting);
    end

    function times_num_mono(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get([1]);
        cx = 5 .* x;
        testCase.assertTrue(isa(cx, 'MTKMonomial'));
        testCase.verifyEqual(cx.Operators, x.Operators);
        testCase.verifyEqual(cx.Coefficient, 5 * x.Coefficient);
    end

    function times_mono_num(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get([1]);
        cx = x .* 5;
        testCase.assertTrue(isa(cx, 'MTKMonomial'));
        testCase.verifyEqual(cx.Operators, x.Operators);
        testCase.verifyEqual(cx.Coefficient, 5 * x.Coefficient);
    end

    function times_mono_mono(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();
        xy_direct = setting.get([1 2]);

        xy = x .* y;
        testCase.assertTrue(isa(xy, 'MTKMonomial'));
        testCase.verifyEqual(xy.Operators, xy_direct.Operators);
        testCase.verifyEqual(xy.Coefficient, xy_direct.Coefficient);
    end

    function times_mono_mono_same(testCase)
        setting = AlgebraicScenario(2);
        [x, ~] = setting.getAll();
        xx_direct = setting.get([1 1]);

        xx = x .* x;
        testCase.assertTrue(isa(xx, 'MTKMonomial'));
        testCase.verifyEqual(xx.Operators, xx_direct.Operators);
        testCase.verifyEqual(xx.Coefficient, xx_direct.Coefficient);
    end


    function times_elementwise(testCase)
        setting = AlgebraicScenario(4);
        [w, x, y, z] = setting.getAll();
        wy_direct = setting.get([1 3]);
        xz_direct = setting.get([2 4]);

        vec_wx = [w;x];
        vec_yz = [y;z];
        the_prod = vec_wx .* vec_yz;

        testCase.assertTrue(isa(the_prod, 'MTKMonomial'));
        testCase.assertEqual(size(the_prod), [2, 1]);
        testCase.verifyTrue(the_prod(1) == wy_direct);
        testCase.verifyTrue(the_prod(2) == xz_direct);
    end

    function times_elementwise_scale(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();
        vec_xy = [x;y];
        nums = [10; 100];

        left_prod = vec_xy .* nums;

        testCase.assertTrue(isa(left_prod, 'MTKMonomial'));
        testCase.assertEqual(size(left_prod), [2, 1]);
        testCase.verifyTrue(left_prod(1) == 10 .* x);
        testCase.verifyTrue(left_prod(2) == 100 .* y);

        right_prod = nums .* vec_xy;

        testCase.verifyTrue(isequal(left_prod, right_prod));
    end

end

%% Conjugate-transpose (ctranspose)
methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'algebraic', 'ctranspose'})
    function ctranspose_id(testCase)
        setting = AlgebraicScenario(2);
        x = setting.id();
        ct_x = x';
        testCase.assertTrue(isa(ct_x, 'MTKMonomial'));
        testCase.verifyEqual(ct_x.Operators, uint64.empty(1,0));
        testCase.verifyEqual(ct_x.Coefficient, 1);
    end

    function ctranspose_mono_hermitian(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get([1 2 2]);
        ct_x = x';
        testCase.assertTrue(isa(ct_x, 'MTKMonomial'));
        testCase.verifyEqual(ct_x.Operators, uint64([2 2 1]));
        testCase.verifyEqual(ct_x.Coefficient, x.Coefficient);
    end

    function ctranspose_mono_nonhermitian(testCase)
        setting = AlgebraicScenario(2, 'hermitian', false,...
            'interleave', false);
        x = setting.get([1 2 2]);
        ct_x = x';
        testCase.assertTrue(isa(ct_x, 'MTKMonomial'));
        testCase.verifyEqual(ct_x.Operators, uint64([4 4 3]));
        testCase.verifyEqual(ct_x.Coefficient, x.Coefficient);
    end
    
    function ctranspose_mono_vector(testCase)
        setting = AlgebraicScenario(3);
        [x,y,z] = setting.getAll();
        wl = setting.WordList(1);
        ct_wl = wl';
        testCase.assertEqual(size(wl), [4 1]);
        testCase.assertTrue(isa(ct_wl, 'MTKMonomial'));
        testCase.assertEqual(size(ct_wl), [1 4]);
        testCase.verifyTrue(ct_wl(1,1) == 1.0);
        testCase.verifyTrue(ct_wl(1,2) == x);
        testCase.verifyTrue(ct_wl(1,3) == y);
        testCase.verifyTrue(ct_wl(1,4) == z);
    end
end

end