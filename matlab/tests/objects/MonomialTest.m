classdef MonomialTest < MTKTestBase
%MONOMIALTEST Tests for MTKMonomial.

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
        testCase.verifyTrue(isa(x_plus_xy, 'MTKPolynomial'));
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