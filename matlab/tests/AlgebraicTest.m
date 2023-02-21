classdef AlgebraicTest < MTKTestBase
%ALGEBRAICTEST Tests for Algebraic.Monomial and Algebraic.Polynomial
    
%% Construction / binding
    methods(Test)
       function zero(testCase)
            setting = AlgebraicScenario(2);
            zero = Algebraic.Polynomial.Zero(setting);
            testCase.verifyTrue(isa(zero, 'Algebraic.Polynomial'));
            testCase.verifyEqual(zero.Scenario, setting);
            testCase.verifyEqual(length(zero.Constituents), 0);           
        end
        
        function get_id(testCase)
            setting = AlgebraicScenario(2);
            id = setting.get([]);
            testCase.verifyTrue(isa(id, 'Algebraic.Monomial'));
            testCase.verifyEqual(id.Operators, uint64.empty(1,0));
            testCase.verifyEqual(id.Coefficient, 1.0);
        end
        
        function get_x(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get([1]);
            testCase.verifyTrue(isa(x, 'Algebraic.Monomial'));
            testCase.verifyEqual(x.Operators, uint64([1]));
            testCase.verifyEqual(x.Coefficient, 1.0);    
        end
        
        function get_xx(testCase)
            setting = AlgebraicScenario(2);
            xx = setting.get([1, 1]);
            testCase.verifyTrue(isa(xx, 'Algebraic.Monomial'));
            testCase.verifyEqual(xx.Operators, uint64([1, 1]));
            testCase.verifyEqual(xx.Coefficient, 1.0);    
        end
        
        function get_xy(testCase)
            setting = AlgebraicScenario(2);
            xy = setting.get([1, 2]);
            testCase.verifyTrue(isa(xy, 'Algebraic.Monomial'));
            testCase.verifyEqual(xy.Operators, uint64([1, 2]));
            testCase.verifyEqual(xy.Coefficient, 1.0);    
        end
        
        function getAll(testCase)
            setting = AlgebraicScenario(3);
            [x, y, z] = setting.getAll();
            testCase.verifyTrue(isa(x, 'Algebraic.Monomial'));
            testCase.verifyTrue(isa(y, 'Algebraic.Monomial'));
            testCase.verifyTrue(isa(z, 'Algebraic.Monomial'));
            testCase.verifyEqual(x.Operators, uint64([1]));
            testCase.verifyEqual(y.Operators, uint64([2]));
            testCase.verifyEqual(z.Operators, uint64([3]));
            testCase.verifyEqual(x.Coefficient, 1.0);
            testCase.verifyEqual(y.Coefficient, 1.0);
            testCase.verifyEqual(z.Coefficient, 1.0);
        end
        
        function id(testCase)
            setting = AlgebraicScenario(2);
            id = setting.id();
            testCase.verifyTrue(isa(id, 'Algebraic.Monomial'));
            testCase.verifyEqual(id.Operators, uint64.empty(1,0));
            testCase.verifyEqual(id.Coefficient, 1.0);
        end
        
        function getAll_and_bind(testCase)
            setting = AlgebraicScenario(3);
            [x, y, z] = setting.getAll();
            mm = setting.MakeMomentMatrix(1);
            testCase.verifyTrue(isa(x, 'Algebraic.Monomial'));
            testCase.verifyTrue(isa(y, 'Algebraic.Monomial'));
            testCase.verifyTrue(isa(z, 'Algebraic.Monomial'));
            testCase.verifyEqual(x.Operators, uint64([1]));
            testCase.verifyEqual(y.Operators, uint64([2]));
            testCase.verifyEqual(z.Operators, uint64([3]));
            testCase.verifyEqual(x.Coefficient, 1.0);
            testCase.verifyEqual(y.Coefficient, 1.0);
            testCase.verifyEqual(z.Coefficient, 1.0);
            testCase.verifyTrue(x.FoundSymbol);
            testCase.verifyTrue(y.FoundSymbol);
            testCase.verifyTrue(z.FoundSymbol);
            testCase.verifyNotEqual(x.SymbolId, y.SymbolId);
            testCase.verifyNotEqual(x.SymbolId, z.SymbolId);
            testCase.verifyNotEqual(y.SymbolId, z.SymbolId);
        end
    end
  
%% Unary plus (uplus)
    methods(Test)
        function uplus_mono(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get(1);
            i_x = +x;
            
            testCase.verifyTrue(isa(i_x, 'Algebraic.Monomial'));
            testCase.assertEqual(i_x.Operators, uint64([1]));
            testCase.assertEqual(i_x.Coefficient, 1.0);
        end
        
        function uplus_poly(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get(1);
            y = setting.get(2);
            x_y = x+y;            
            ux_y = +x_y;
            
            testCase.verifyTrue(isa(ux_y, 'Algebraic.Polynomial'));

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
    methods(Test)
        function uminus_mono(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get(1);
            i_x = -x;
            
            testCase.verifyTrue(isa(i_x, 'Algebraic.Monomial'));
            testCase.assertEqual(i_x.Operators, uint64([1]));
            testCase.assertEqual(i_x.Coefficient, -1.0);
        end
        
        function uminus_poly(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get(1);
            y = setting.get(2);
            x_y = x+y;            
            ux_y = -x_y;
            
            testCase.verifyTrue(isa(ux_y, 'Algebraic.Polynomial'));

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
    methods(Test)
        function plus_num_mono(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get(1);
            i_x = 1 + x;
            
            testCase.verifyTrue(isa(i_x, 'Algebraic.Polynomial'));
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
            
            testCase.verifyTrue(isa(x_i, 'Algebraic.Polynomial'));
            testCase.assertEqual(length(x_i.Constituents), 2);
            part_i = x_i.Constituents(1);
            part_x = x_i.Constituents(2);
            testCase.assertEqual(part_i.Operators, uint64.empty(1,0));
            testCase.assertEqual(part_i.Coefficient, 1.0);
            testCase.assertEqual(part_x.Operators, x.Operators);
            testCase.assertEqual(part_x.Coefficient, x.Coefficient);
        end
        
        function plus_mono_mono(testCase)
            setting = AlgebraicScenario(2);
            [x, y] = setting.getAll();
            
            x_plus_y = x + y;
            y_plus_x = y + x;
            testCase.verifyTrue(isa(x_plus_y, 'Algebraic.Polynomial'));
            testCase.assertEqual(length(x_plus_y.Constituents), 2);
            part_x = x_plus_y.Constituents(1);
            part_y = x_plus_y.Constituents(2);
            testCase.assertEqual(part_x.Operators, x.Operators);
            testCase.assertEqual(part_x.Coefficient, x.Coefficient);
            testCase.assertEqual(part_y.Operators, y.Operators);
            testCase.assertEqual(part_y.Coefficient, y.Coefficient);
            
            testCase.verifyTrue(isa(y_plus_x, 'Algebraic.Polynomial'));
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
            testCase.verifyTrue(isa(x_plus_x, 'Algebraic.Monomial'));
            testCase.verifyEqual(x_plus_x.Operators, x.Operators);
            testCase.verifyEqual(x_plus_x.Coefficient, 2 * x.Coefficient);
        end
        
          
        function plus_mono_mono_different_length(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get([1]);
            xy = setting.get([1, 2]);
            
            x_plus_xy = x + xy;
            xy_plus_x = xy + x;
            testCase.verifyTrue(isa(x_plus_xy, 'Algebraic.Polynomial'));
            testCase.assertEqual(length(x_plus_xy.Constituents), 2);
            part_x = x_plus_xy.Constituents(1);
            part_xy = x_plus_xy.Constituents(2);            
            testCase.assertEqual(part_x.Operators, x.Operators);
            testCase.assertEqual(part_x.Coefficient, x.Coefficient);
            testCase.assertEqual(part_xy.Operators, xy.Operators);
            testCase.assertEqual(part_xy.Coefficient, xy.Coefficient);            
            
            testCase.verifyTrue(isa(xy_plus_x, 'Algebraic.Polynomial'));
            testCase.assertEqual(length(xy_plus_x.Constituents), 2);
            partB_x = xy_plus_x.Constituents(1);
            partB_xy = xy_plus_x.Constituents(2);                       
            testCase.assertEqual(partB_x.Operators, x.Operators);
            testCase.assertEqual(partB_x.Coefficient, x.Coefficient);
            testCase.assertEqual(partB_xy.Operators, xy.Operators);
            testCase.assertEqual(partB_xy.Coefficient, xy.Coefficient);
        end
        
        function plus_mono_poly(testCase)
            setting = AlgebraicScenario(3);
            [x, y, z] = setting.getAll();
            
            y_z = y + z;
            testCase.verifyTrue(isa(y_z , 'Algebraic.Polynomial'));
            x_y_z = x + y_z;
            testCase.verifyTrue(isa(x_y_z , 'Algebraic.Polynomial'));
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
            testCase.verifyTrue(isa(x_y , 'Algebraic.Polynomial'));
            
            x_x_y = x + x_y;
            testCase.verifyTrue(isa(x_x_y , 'Algebraic.Polynomial'));
            testCase.verifyTrue(isa(x_x_y , 'Algebraic.Polynomial'));
            testCase.assertEqual(length(x_x_y.Constituents), 2);
            partA_x = x_x_y.Constituents(1);
            partA_y = x_x_y.Constituents(2);
            
            testCase.verifyEqual(partA_x.Operators, x.Operators);
            testCase.verifyEqual(partA_x.Coefficient, 2 * x.Coefficient);
            testCase.verifyEqual(partA_y.Operators, y.Operators);
            testCase.verifyEqual(partA_y.Coefficient, y.Coefficient);
            
            y_x_y = y + x_y;
            testCase.verifyTrue(isa(y_x_y , 'Algebraic.Polynomial'));
            testCase.verifyTrue(isa(y_x_y , 'Algebraic.Polynomial'));
            testCase.assertEqual(length(y_x_y.Constituents), 2);
            partB_x = y_x_y.Constituents(1);
            partB_y = y_x_y.Constituents(2);
            
            testCase.verifyEqual(partB_x.Operators, x.Operators);
            testCase.verifyEqual(partB_x.Coefficient, x.Coefficient);
            testCase.verifyEqual(partB_y.Operators, y.Operators);
            testCase.verifyEqual(partB_y.Coefficient, 2 * y.Coefficient);
        end
        
        function plus_poly_mono_overlap(testCase)
            setting = AlgebraicScenario(2);
            [x, y] = setting.getAll();
            
            x_y = x + y;
            testCase.verifyTrue(isa(x_y , 'Algebraic.Polynomial'));
            
            x_y_x = x_y + x;
            testCase.verifyTrue(isa(x_y_x , 'Algebraic.Polynomial'));
            testCase.verifyTrue(isa(x_y_x , 'Algebraic.Polynomial'));
            testCase.assertEqual(length(x_y_x.Constituents), 2);
            partA_x = x_y_x.Constituents(1);
            partA_y = x_y_x.Constituents(2);
            
            testCase.verifyEqual(partA_x.Operators, x.Operators);
            testCase.verifyEqual(partA_x.Coefficient, 2 * x.Coefficient);
            testCase.verifyEqual(partA_y.Operators, y.Operators);
            testCase.verifyEqual(partA_y.Coefficient, y.Coefficient);
            
            x_y_y = x_y + y;
            testCase.verifyTrue(isa(x_y_y , 'Algebraic.Polynomial'));
            testCase.verifyTrue(isa(x_y_y , 'Algebraic.Polynomial'));
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
            testCase.verifyTrue(isa(x_y, 'Algebraic.Polynomial'));
            
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
            testCase.verifyTrue(isa(x_y, 'Algebraic.Polynomial'));
            
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
            
            testCase.assertTrue(isa(w_x, 'Algebraic.Polynomial'));
            testCase.assertTrue(isa(y_z, 'Algebraic.Polynomial'));
            
            w_x_y_z = w_x + y_z;
            testCase.assertTrue(isa(w_x_y_z, 'Algebraic.Polynomial'));
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
            testCase.assertTrue(isa(y_z_w_x, 'Algebraic.Polynomial'));
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
            testCase.assertTrue(isa(x_2y_z, 'Algebraic.Polynomial'));
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
            testCase.assertTrue(isa(alt_x_2y_z, 'Algebraic.Polynomial'));
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
    methods(Test)
        function minus_num_mono(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get(1);
            i_x = 1 - x;
            
            testCase.verifyTrue(isa(i_x, 'Algebraic.Polynomial'));
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
            
            testCase.verifyTrue(isa(x_i, 'Algebraic.Polynomial'));
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
            testCase.verifyTrue(isa(x_minus_y, 'Algebraic.Polynomial'));
            testCase.assertEqual(length(x_minus_y.Constituents), 2);
            part_x = x_minus_y.Constituents(1);
            part_y = x_minus_y.Constituents(2);
            testCase.assertEqual(part_x.Operators, x.Operators);
            testCase.assertEqual(part_x.Coefficient, x.Coefficient);
            testCase.assertEqual(part_y.Operators, y.Operators);
            testCase.assertEqual(part_y.Coefficient, -y.Coefficient);
            
            testCase.verifyTrue(isa(y_minus_x, 'Algebraic.Polynomial'));
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
            testCase.verifyTrue(isa(x_minus_x, 'double'));
            testCase.verifyEqual(x_minus_x, 0);
        end
         
        function minus_mono_poly(testCase)
            setting = AlgebraicScenario(3);
            [x, y, z] = setting.getAll();
            
            y_z = y + z;
            testCase.verifyTrue(isa(y_z , 'Algebraic.Polynomial'));
            x_y_z = x - y_z;
            testCase.verifyTrue(isa(x_y_z , 'Algebraic.Polynomial'));
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
            testCase.verifyTrue(isa(x_y , 'Algebraic.Polynomial'));
            
            x_x_y = x - 2*x_y;
            testCase.verifyTrue(isa(x_x_y , 'Algebraic.Polynomial'));
            testCase.verifyTrue(isa(x_x_y , 'Algebraic.Polynomial'));
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
            testCase.verifyTrue(isa(x_y , 'Algebraic.Polynomial'));
            
            alt_y = x - x_y;
            testCase.assertTrue(isa(alt_y , 'Algebraic.Monomial'));
            
            testCase.verifyEqual(alt_y.Operators, y.Operators);
            testCase.verifyEqual(alt_y.Coefficient, -y.Coefficient);            
        end

        function minus_poly_poly(testCase)
            setting = AlgebraicScenario(4);
            [w, x, y, z] = setting.getAll();
            
            w_x = w + x;
            y_z = y + z;
            
            testCase.assertTrue(isa(w_x, 'Algebraic.Polynomial'));
            testCase.assertTrue(isa(y_z, 'Algebraic.Polynomial'));
            
            w_x_y_z = w_x - y_z;
            testCase.assertTrue(isa(w_x_y_z, 'Algebraic.Polynomial'));
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
            testCase.assertTrue(isa(x_2y_z, 'Algebraic.Polynomial'));
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
            testCase.assertTrue(isa(x_2y_z, 'Algebraic.Polynomial'));
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
            
            testCase.assertTrue(isa(as_zero, 'double'));
            testCase.verifyEqual(as_zero, 0);
  
        end
    end
    
%% Multiplication (mtimes)
    methods(Test)
        function mtimes_num_mono(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get([1]);
            cx = 5 * x;
            testCase.assertTrue(isa(cx, 'Algebraic.Monomial'));
            testCase.verifyEqual(cx.Operators, x.Operators);
            testCase.verifyEqual(cx.Coefficient, 5 * x.Coefficient);
        end
        
        function mtimes_mono_num(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get([1]);
            cx = x * 5;
            testCase.assertTrue(isa(cx, 'Algebraic.Monomial'));
            testCase.verifyEqual(cx.Operators, x.Operators);
            testCase.verifyEqual(cx.Coefficient, 5 * x.Coefficient);
        end
        
        function mtimes_mono_mono(testCase)
            setting = AlgebraicScenario(2);
            [x, y] = setting.getAll();
            xy_direct = setting.get([1 2]);
            
            xy = x * y;
            testCase.assertTrue(isa(xy, 'Algebraic.Monomial'));
            testCase.verifyEqual(xy.Operators, xy_direct.Operators);
            testCase.verifyEqual(xy.Coefficient, xy_direct.Coefficient);
        end
        
        function mtimes_mono_mono_same(testCase)
            setting = AlgebraicScenario(2);
            [x, ~] = setting.getAll();
            xx_direct = setting.get([1 1]);
            
            xx = x * x;
            testCase.assertTrue(isa(xx, 'Algebraic.Monomial'));
            testCase.verifyEqual(xx.Operators, xx_direct.Operators);
            testCase.verifyEqual(xx.Coefficient, xx_direct.Coefficient);
        end
            
        function mtimes_mono_poly(testCase)
            setting = AlgebraicScenario(3);
            [x, y, z] = setting.getAll();
            
            xy_direct = setting.get([1, 2]);
            xz_direct = setting.get([1, 3]);
            
            y_z = y + z;
            testCase.assertTrue(isa(y_z, 'Algebraic.Polynomial'));
            
            xy_xz = x * y_z;
            testCase.assertTrue(isa(xy_xz, 'Algebraic.Polynomial'));
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
            testCase.assertTrue(isa(x_y , 'Algebraic.Polynomial'));
            
            xz_yz = x_y * z;
            testCase.assertTrue(isa(xz_yz, 'Algebraic.Polynomial'));
            testCase.assertEqual(length(xz_yz.Constituents), 2);
            
            xz = xz_yz.Constituents(1);
            yz = xz_yz.Constituents(2);
                                   
            testCase.verifyEqual(xz.Operators, xz_direct.Operators);
            testCase.verifyEqual(xz.Coefficient, xz_direct.Coefficient);            
            testCase.verifyEqual(yz.Operators, yz_direct.Operators);
            testCase.verifyEqual(yz.Coefficient, yz_direct.Coefficient);
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
            testCase.assertTrue(isa(w_x, 'Algebraic.Polynomial'));
            testCase.assertTrue(isa(y_z, 'Algebraic.Polynomial'));
            
            wy_wz_xy_xz = w_x * y_z;
            testCase.assertTrue(isa(wy_wz_xy_xz, 'Algebraic.Polynomial'));
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

%% Complex conjugation (ctranspose)
    methods(Test)
        function ctranspose_id(testCase)
            setting = AlgebraicScenario(2);
            x = setting.id();
            ct_x = x';
            testCase.assertTrue(isa(ct_x, 'Algebraic.Monomial'));
            testCase.verifyEqual(ct_x.Operators, uint64.empty(1,0));
            testCase.verifyEqual(ct_x.Coefficient, 1);
        end  
        
        function ctranspose_mono_hermitian(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get([1 2 2]);
            ct_x = x';
            testCase.assertTrue(isa(ct_x, 'Algebraic.Monomial'));
            testCase.verifyEqual(ct_x.Operators, uint64([2 2 1]));
            testCase.verifyEqual(ct_x.Coefficient, x.Coefficient);
        end  
        
        function ctranspose_mono_nonhermitian(testCase)
            setting = AlgebraicScenario(2, {}, false);
            x = setting.get([1 2 2]);
            ct_x = x';
            testCase.assertTrue(isa(ct_x, 'Algebraic.Monomial'));
            testCase.verifyEqual(ct_x.Operators, uint64([4 4 3]));
            testCase.verifyEqual(ct_x.Coefficient, x.Coefficient);
        end  
        
       function ctranspose_poly_hermitian(testCase)
            setting = AlgebraicScenario(2, {}, false);
            x = setting.get([1 2]);
            y = setting.get([2]);
            x_plus_y = x + y;            
            ct = x_plus_y';
            testCase.assertTrue(isa(ct, 'Algebraic.Polynomial'));
            testCase.assertEqual(length(ct.Constituents), 2);
            yc = ct.Constituents(1);
            yc_xc = ct.Constituents(2);            
            testCase.verifyEqual(yc.Operators, uint64([4]));
            testCase.verifyEqual(yc_xc.Operators, uint64([4 3]));
            
        end  
    end
    
end

