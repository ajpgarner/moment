classdef PolynomialTest < MTKTestBase
%POLYNOMIALTEST Tests for MTKPolynomial.
        
%% Equality (eq)
methods(Test, TestTags={'MTKPolynomial', 'MTKObject', 'eq'})
    function eq_poly_poly(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();
        p1 = x + 2*y;
        p2 = 2*x + y;
        p3 = 2*y + x;
        testCase.verifyFalse(p1 == p2);
        testCase.verifyFalse(p2 == p1);
        testCase.verifyTrue(p1 == p3);
        testCase.verifyTrue(p3 == p1);
        testCase.verifyFalse(p2 == p3);
        testCase.verifyFalse(p3 == p2);            
    end
end

%% Unary plus (uplus)
methods(Test, TestTags={'MTKPolynomial', 'MTKObject', 'algebraic', 'uplus'})
      function uplus_poly(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        y = setting.get(2);
        x_y = x+y;            
        ux_y = +x_y;

        testCase.verifyTrue(isa(ux_y, 'MTKPolynomial'));

        testCase.assertEqual(length(ux_y.Constituents), 2);
        part_x = ux_y.Constituents(1);
        part_y = ux_y.Constituents(2);            
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
        testCase.assertEqual(part_y.Operators, y.Operators);
        testCase.assertEqual(part_y.Coefficient, y.Coefficient);
    end
end

%% Unary minus (uminus)
methods(Test, TestTags={'MTKPolynomial', 'MTKObject', 'algebraic', 'uminus'})
    function uminus_poly(testCase)
        setting = AlgebraicScenario(2);
        x = setting.get(1);
        y = setting.get(2);
        x_y = x+y;            
        ux_y = -x_y;

        testCase.verifyTrue(isa(ux_y, 'MTKPolynomial'));

        testCase.assertEqual(length(ux_y.Constituents), 2);
        part_x = ux_y.Constituents(1);
        part_y = ux_y.Constituents(2);            
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, -x.Coefficient);
        testCase.assertEqual(part_y.Operators, y.Operators);
        testCase.assertEqual(part_y.Coefficient, -y.Coefficient);
    end
end

%% Addition (plus)
methods(Test, TestTags={'MTKPolynomial', 'MTKObject', 'algebraic', 'plus'})

    function plus_mono_poly(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();

        y_z = y + z;
        testCase.verifyTrue(isa(y_z , 'MTKPolynomial'));
        x_y_z = x + y_z;
        testCase.verifyTrue(isa(x_y_z , 'MTKPolynomial'));
        testCase.assertEqual(length(x_y_z.Constituents), 3);
        part_x = x_y_z.Constituents(1);
        part_y = x_y_z.Constituents(2);
        part_z = x_y_z.Constituents(3);

        testCase.verifyEqual(part_x.Operators, x.Operators);
        testCase.verifyEqual(part_x.Coefficient, x.Coefficient);
        testCase.verifyEqual(part_y.Operators, y.Operators);
        testCase.verifyEqual(part_y.Coefficient, y.Coefficient);
        testCase.verifyEqual(part_z.Operators, z.Operators);
        testCase.verifyEqual(part_z.Coefficient, z.Coefficient);
    end
        
        
    function plus_mono_poly_overlap(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_y = x + y;
        testCase.verifyTrue(isa(x_y , 'MTKPolynomial'));

        x_x_y = x + x_y;
        testCase.verifyTrue(isa(x_x_y , 'MTKPolynomial'));
        testCase.verifyTrue(isa(x_x_y , 'MTKPolynomial'));
        testCase.assertEqual(length(x_x_y.Constituents), 2);
        partA_x = x_x_y.Constituents(1);
        partA_y = x_x_y.Constituents(2);

        testCase.verifyEqual(partA_x.Operators, x.Operators);
        testCase.verifyEqual(partA_x.Coefficient, 2 * x.Coefficient);
        testCase.verifyEqual(partA_y.Operators, y.Operators);
        testCase.verifyEqual(partA_y.Coefficient, y.Coefficient);

        y_x_y = y + x_y;
        testCase.verifyTrue(isa(y_x_y , 'MTKPolynomial'));
        testCase.verifyTrue(isa(y_x_y , 'MTKPolynomial'));
        testCase.assertEqual(length(y_x_y.Constituents), 2);
        partB_x = y_x_y.Constituents(1);
        partB_y = y_x_y.Constituents(2);

        testCase.verifyEqual(partB_x.Operators, x.Operators);
        testCase.verifyEqual(partB_x.Coefficient, x.Coefficient);
        testCase.verifyEqual(partB_y.Operators, y.Operators);
        testCase.verifyEqual(partB_y.Coefficient, 2 * y.Coefficient);
    end

    function plus_zero_poly(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero();           
        [x, y] = setting.getAll();
        x_plus_y = x + y;           
        also_x_plus_y = zero + x_plus_y;
        testCase.verifyTrue(eq(also_x_plus_y, x_plus_y));
    end

    function plus_poly_zero(testCase)
        setting = AlgebraicScenario(2);
        zero = setting.zero();           
        [x, y] = setting.getAll();
        x_plus_y = x + y;           
        also_x_plus_y = x_plus_y + zero;
        testCase.verifyTrue(eq(also_x_plus_y, x_plus_y));
    end


    function plus_poly_mono_overlap(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_y = x + y;
        testCase.verifyTrue(isa(x_y , 'MTKPolynomial'));

        x_y_x = x_y + x;
        testCase.verifyTrue(isa(x_y_x , 'MTKPolynomial'));
        testCase.verifyTrue(isa(x_y_x , 'MTKPolynomial'));
        testCase.assertEqual(length(x_y_x.Constituents), 2);
        partA_x = x_y_x.Constituents(1);
        partA_y = x_y_x.Constituents(2);

        testCase.verifyEqual(partA_x.Operators, x.Operators);
        testCase.verifyEqual(partA_x.Coefficient, 2 * x.Coefficient);
        testCase.verifyEqual(partA_y.Operators, y.Operators);
        testCase.verifyEqual(partA_y.Coefficient, y.Coefficient);

        x_y_y = x_y + y;
        testCase.verifyTrue(isa(x_y_y , 'MTKPolynomial'));
        testCase.verifyTrue(isa(x_y_y , 'MTKPolynomial'));
        testCase.assertEqual(length(x_y_y.Constituents), 2);
        partB_x = x_y_y.Constituents(1);
        partB_y = x_y_y.Constituents(2);

        testCase.verifyEqual(partB_x.Operators, x.Operators);
        testCase.verifyEqual(partB_x.Coefficient, x.Coefficient);
        testCase.verifyEqual(partB_y.Operators, y.Operators);
        testCase.verifyEqual(partB_y.Coefficient, 2 * y.Coefficient);
    end

    function plus_num_poly(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_y = x + y;
        testCase.verifyTrue(isa(x_y, 'MTKPolynomial'));

        i_x_y = 1 + x_y;
        testCase.assertEqual(length(i_x_y.Constituents), 3);
        part_i = i_x_y.Constituents(1);
        part_x = i_x_y.Constituents(2);
        part_y = i_x_y.Constituents(3);

        testCase.verifyEqual(part_i.Operators, uint64.empty(1,0));
        testCase.verifyEqual(part_i.Coefficient, 1.0);
        testCase.verifyEqual(part_x.Operators, x.Operators);
        testCase.verifyEqual(part_x.Coefficient, 1.0);
        testCase.verifyEqual(part_y.Operators, y.Operators);
        testCase.verifyEqual(part_y.Coefficient, 1.0);
    end

    function plus_poly_num(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_y = x + y;
        testCase.verifyTrue(isa(x_y, 'MTKPolynomial'));

        x_y_i = x_y + 1;
        testCase.assertEqual(length(x_y_i.Constituents), 3);
        part_i = x_y_i.Constituents(1);
        part_x = x_y_i.Constituents(2);
        part_y = x_y_i.Constituents(3);

        testCase.verifyEqual(part_i.Operators, uint64.empty(1,0));
        testCase.verifyEqual(part_i.Coefficient, 1.0);
        testCase.verifyEqual(part_x.Operators, x.Operators);
        testCase.verifyEqual(part_x.Coefficient, 1.0);
        testCase.verifyEqual(part_y.Operators, y.Operators);
        testCase.verifyEqual(part_y.Coefficient, 1.0);
    end

    function plus_poly_poly(testCase)
        setting = AlgebraicScenario(4);
        [w, x, y, z] = setting.getAll();

        w_x = w + x;
        y_z = y + z;

        testCase.assertTrue(isa(w_x, 'MTKPolynomial'));
        testCase.assertTrue(isa(y_z, 'MTKPolynomial'));

        w_x_y_z = w_x + y_z;
        testCase.assertTrue(isa(w_x_y_z, 'MTKPolynomial'));
        testCase.assertEqual(length(w_x_y_z.Constituents), 4);
        part_w = w_x_y_z.Constituents(1);
        part_x = w_x_y_z.Constituents(2);
        part_y = w_x_y_z.Constituents(3);
        part_z = w_x_y_z.Constituents(4);

        testCase.assertEqual(part_w.Operators, w.Operators);
        testCase.assertEqual(part_w.Coefficient, w.Coefficient);
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
        testCase.assertEqual(part_y.Operators, y.Operators);
        testCase.assertEqual(part_y.Coefficient, y.Coefficient);
        testCase.assertEqual(part_z.Operators, z.Operators);
        testCase.assertEqual(part_z.Coefficient, z.Coefficient);

        y_z_w_x = y_z + w_x;
        testCase.assertTrue(isa(y_z_w_x, 'MTKPolynomial'));
        testCase.assertEqual(length(y_z_w_x.Constituents), 4);
        partB_w = y_z_w_x.Constituents(1);
        partB_x = y_z_w_x.Constituents(2);
        partB_y = y_z_w_x.Constituents(3);
        partB_z = y_z_w_x.Constituents(4);

        testCase.assertEqual(partB_w.Operators, w.Operators);
        testCase.assertEqual(partB_w.Coefficient, w.Coefficient);
        testCase.assertEqual(partB_x.Operators, x.Operators);
        testCase.assertEqual(partB_x.Coefficient, x.Coefficient);
        testCase.assertEqual(partB_y.Operators, y.Operators);
        testCase.assertEqual(partB_y.Coefficient, y.Coefficient);
        testCase.assertEqual(partB_z.Operators, z.Operators);
        testCase.assertEqual(partB_z.Coefficient, z.Coefficient);
    end

    function plus_poly_poly_overlap(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();

        x_y = x + y;
        y_z = y + z;
        x_2y_z = x_y + y_z;            
        testCase.assertTrue(isa(x_2y_z, 'MTKPolynomial'));
        testCase.assertEqual(length(x_2y_z.Constituents), 3);

        partA_x = x_2y_z.Constituents(1);
        partA_y = x_2y_z.Constituents(2);
        partA_z = x_2y_z.Constituents(3);

        testCase.assertEqual(partA_x.Operators, x.Operators);
        testCase.assertEqual(partA_x.Coefficient, x.Coefficient);
        testCase.assertEqual(partA_y.Operators, y.Operators);
        testCase.assertEqual(partA_y.Coefficient, 2.0 * y.Coefficient);
        testCase.assertEqual(partA_z.Operators, z.Operators);
        testCase.assertEqual(partA_z.Coefficient, z.Coefficient);

        alt_x_2y_z = y_z + x_y; 
        testCase.assertTrue(isa(alt_x_2y_z, 'MTKPolynomial'));
        testCase.assertEqual(length(alt_x_2y_z.Constituents), 3);

        partB_x = alt_x_2y_z.Constituents(1);
        partB_y = alt_x_2y_z.Constituents(2);
        partB_z = alt_x_2y_z.Constituents(3);

        testCase.assertEqual(partB_x.Operators, x.Operators);
        testCase.assertEqual(partB_x.Coefficient, x.Coefficient);
        testCase.assertEqual(partB_y.Operators, y.Operators);
        testCase.assertEqual(partB_y.Coefficient, 2.0 * y.Coefficient);
        testCase.assertEqual(partB_z.Operators, z.Operators);
        testCase.assertEqual(partB_z.Coefficient, z.Coefficient);
    end
end  

%% Subtraction (minus)
methods(Test, TestTags={'MTKPolynomial', 'MTKObject', 'algebraic', 'minus'})

    function minus_mono_poly(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();

        y_z = y + z;
        testCase.verifyTrue(isa(y_z , 'MTKPolynomial'));
        x_y_z = x - y_z;
        testCase.verifyTrue(isa(x_y_z , 'MTKPolynomial'));
        testCase.assertEqual(length(x_y_z.Constituents), 3);
        part_x = x_y_z.Constituents(1);
        part_y = x_y_z.Constituents(2);
        part_z = x_y_z.Constituents(3);

        testCase.verifyEqual(part_x.Operators, x.Operators);
        testCase.verifyEqual(part_x.Coefficient, x.Coefficient);
        testCase.verifyEqual(part_y.Operators, y.Operators);
        testCase.verifyEqual(part_y.Coefficient, -y.Coefficient);
        testCase.verifyEqual(part_z.Operators, z.Operators);
        testCase.verifyEqual(part_z.Coefficient, -z.Coefficient);
    end

    function minus_mono_poly_overlap(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_y = x + y;
        testCase.verifyTrue(isa(x_y , 'MTKPolynomial'));

        x_x_y = x - 2*x_y;
        testCase.verifyTrue(isa(x_x_y , 'MTKPolynomial'));
        testCase.verifyTrue(isa(x_x_y , 'MTKPolynomial'));
        testCase.assertEqual(length(x_x_y.Constituents), 2);
        partA_x = x_x_y.Constituents(1);
        partA_y = x_x_y.Constituents(2);

        testCase.verifyEqual(partA_x.Operators, x.Operators);
        testCase.verifyEqual(partA_x.Coefficient, -x.Coefficient);
        testCase.verifyEqual(partA_y.Operators, y.Operators);
        testCase.verifyEqual(partA_y.Coefficient, -2*y.Coefficient);
    end


    function minus_mono_poly_overlap_to_zero(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_y = x + y;
        testCase.verifyTrue(isa(x_y , 'MTKPolynomial'));

        alt_y = x - x_y;
        testCase.assertTrue(isa(alt_y , 'MTKPolynomial'));

        testCase.verifyEqual(alt_y.Constituents(1).Operators, y.Operators);
        testCase.verifyEqual(alt_y.Constituents(1).Coefficient, -y.Coefficient);            
    end

    function minus_poly_poly(testCase)
        setting = AlgebraicScenario(4);
        [w, x, y, z] = setting.getAll();

        w_x = w + x;
        y_z = y + z;

        testCase.assertTrue(isa(w_x, 'MTKPolynomial'));
        testCase.assertTrue(isa(y_z, 'MTKPolynomial'));

        w_x_y_z = w_x - y_z;
        testCase.assertTrue(isa(w_x_y_z, 'MTKPolynomial'));
        testCase.assertEqual(length(w_x_y_z.Constituents), 4);
        part_w = w_x_y_z.Constituents(1);
        part_x = w_x_y_z.Constituents(2);
        part_y = w_x_y_z.Constituents(3);
        part_z = w_x_y_z.Constituents(4);

        testCase.assertEqual(part_w.Operators, w.Operators);
        testCase.assertEqual(part_w.Coefficient, w.Coefficient);
        testCase.assertEqual(part_x.Operators, x.Operators);
        testCase.assertEqual(part_x.Coefficient, x.Coefficient);
        testCase.assertEqual(part_y.Operators, y.Operators);
        testCase.assertEqual(part_y.Coefficient, -y.Coefficient);
        testCase.assertEqual(part_z.Operators, z.Operators);
        testCase.assertEqual(part_z.Coefficient, -z.Coefficient);
    end

    function minus_poly_poly_overlap(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();

        x_y = x + y;
        y_z = y + z;
        x_2y_z = x_y - 2*y_z;            
        testCase.assertTrue(isa(x_2y_z, 'MTKPolynomial'));
        testCase.assertEqual(length(x_2y_z.Constituents), 3);

        partA_x = x_2y_z.Constituents(1);
        partA_y = x_2y_z.Constituents(2);
        partA_z = x_2y_z.Constituents(3);

        testCase.assertEqual(partA_x.Operators, x.Operators);
        testCase.assertEqual(partA_x.Coefficient, x.Coefficient);
        testCase.assertEqual(partA_y.Operators, y.Operators);
        testCase.assertEqual(partA_y.Coefficient, -y.Coefficient);
        testCase.assertEqual(partA_z.Operators, z.Operators);
        testCase.assertEqual(partA_z.Coefficient, -2* z.Coefficient);

    end

    function minus_poly_poly_overlap_to_zero(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();

        x_y = x + y;
        y_z = y + z;
        x_2y_z = x_y - y_z;            
        testCase.assertTrue(isa(x_2y_z, 'MTKPolynomial'));
        testCase.assertEqual(length(x_2y_z.Constituents), 2);

        partA_x = x_2y_z.Constituents(1);
        partA_z = x_2y_z.Constituents(2);

        testCase.assertEqual(partA_x.Operators, x.Operators);
        testCase.assertEqual(partA_x.Coefficient, x.Coefficient);
        testCase.assertEqual(partA_z.Operators, z.Operators);
        testCase.assertEqual(partA_z.Coefficient, -z.Coefficient);

    end


    function minus_poly_poly_to_zero(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll();

        x_y = x + y;
        x_y2 = x + y;           

        as_zero = x_y - x_y2;

        testCase.assertTrue(as_zero.IsZero);
        testCase.verifyEqual(as_zero.Scenario, setting);

    end
end

%% Elementwise multiplication (times)
methods(Test, TestTags={'MTKPolynomial', 'MTKObject', 'algebraic', 'times'})
    function mtimes_mono_poly(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();

        xy_direct = setting.get([1, 2]);
        xz_direct = setting.get([1, 3]);

        y_z = y + z;
        testCase.assertTrue(isa(y_z, 'MTKPolynomial'));

        xy_xz = x * y_z;
        testCase.assertTrue(isa(xy_xz, 'MTKPolynomial'));
        testCase.assertEqual(length(xy_xz.Constituents), 2);

        xy = xy_xz.Constituents(1);
        xz = xy_xz.Constituents(2);

        testCase.verifyEqual(xy.Operators, xy_direct.Operators);
        testCase.verifyEqual(xy.Coefficient, xy_direct.Coefficient);            
        testCase.verifyEqual(xz.Operators, xz_direct.Operators);
        testCase.verifyEqual(xz.Coefficient, xz_direct.Coefficient);
    end    

    function mtimes_poly_mono(testCase)
        setting = AlgebraicScenario(3);
        [x, y, z] = setting.getAll();

        xz_direct = setting.get([1, 3]);
        yz_direct = setting.get([2, 3]);

        x_y = x + y;
        testCase.assertTrue(isa(x_y , 'MTKPolynomial'));

        xz_yz = x_y * z;
        testCase.assertTrue(isa(xz_yz, 'MTKPolynomial'));
        testCase.assertEqual(length(xz_yz.Constituents), 2);

        xz = xz_yz.Constituents(1);
        yz = xz_yz.Constituents(2);

        testCase.verifyEqual(xz.Operators, xz_direct.Operators);
        testCase.verifyEqual(xz.Coefficient, xz_direct.Coefficient);            
        testCase.verifyEqual(yz.Operators, yz_direct.Operators);
        testCase.verifyEqual(yz.Coefficient, yz_direct.Coefficient);
    end

    function mtimes_zero_poly(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll;
        x_plus_y = x + y;
        zero = setting.zero();
        also_zero = zero * x_plus_y;
        testCase.assertTrue(also_zero.IsZero);
        testCase.assertTrue(eq(also_zero, zero));
        testCase.verifyEqual(also_zero.Scenario, setting);
    end

    function mtimes_poly_zero(testCase)
        setting = AlgebraicScenario(2);
        [x, y] = setting.getAll;
        x_plus_y = x + y;
        zero = setting.zero();
        also_zero = x_plus_y * zero;
        testCase.assertTrue(also_zero.IsZero);
        testCase.assertTrue(eq(also_zero, zero));
        testCase.verifyEqual(also_zero.Scenario, setting);
    end

    function mtimes_poly_poly(testCase)
        setting = AlgebraicScenario(4);
        [w, x, y, z] = setting.getAll();

        wy_direct = setting.get([1, 3]);
        wz_direct = setting.get([1, 4]);
        xy_direct = setting.get([2, 3]);
        xz_direct = setting.get([2, 4]);

        w_x = w + x;
        y_z = y + z;
        testCase.assertTrue(isa(w_x, 'MTKPolynomial'));
        testCase.assertTrue(isa(y_z, 'MTKPolynomial'));

        wy_wz_xy_xz = w_x * y_z;
        testCase.assertTrue(isa(wy_wz_xy_xz, 'MTKPolynomial'));
        testCase.assertEqual(length(wy_wz_xy_xz.Constituents), 4);

        wy = wy_wz_xy_xz.Constituents(1);
        wz = wy_wz_xy_xz.Constituents(2);
        xy = wy_wz_xy_xz.Constituents(3);
        xz = wy_wz_xy_xz.Constituents(4);

        testCase.verifyEqual(wy.Operators, wy_direct.Operators);
        testCase.verifyEqual(wy.Coefficient, wy_direct.Coefficient);            
        testCase.verifyEqual(wz.Operators, wz_direct.Operators);
        testCase.verifyEqual(wz.Coefficient, wz_direct.Coefficient);            
        testCase.verifyEqual(xy.Operators, xy_direct.Operators);
        testCase.verifyEqual(xy.Coefficient, xy_direct.Coefficient);            
        testCase.verifyEqual(xz.Operators, xz_direct.Operators);
        testCase.verifyEqual(xz.Coefficient, xz_direct.Coefficient);
    end
end

%% Conjugate-transpose (ctranspose)
methods(Test, TestTags={'MTKPolynomial', 'MTKObject', 'algebraic', 'ctranspose'})
   function ctranspose_poly_hermitian(testCase)
        setting = AlgebraicScenario(2, 'hermitian', false, ...
                                       'interleave', false);
        x = setting.get([1 2]);
        y = setting.get([2]);
        x_plus_y = x + y;            
        ct = x_plus_y';
        testCase.assertTrue(isa(ct, 'MTKPolynomial'));
        testCase.assertEqual(length(ct.Constituents), 2);
        yc = ct.Constituents(1);
        yc_xc = ct.Constituents(2);
        testCase.verifyEqual(yc.Operators, uint64([4]));
        testCase.verifyEqual(yc_xc.Operators, uint64([4 3]));

    end  
end
    
end

