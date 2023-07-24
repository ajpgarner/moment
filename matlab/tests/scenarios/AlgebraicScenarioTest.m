classdef AlgebraicScenarioTest < MTKTestBase
%ALGEBRAICTEST Tests for AlgebraicScenario object.
        
%% Construction / binding
    methods(Test, TestTags={'MTKScenario'})
        function get_zero(testCase)
            setting = AlgebraicScenario(2);
            zero = setting.zero();
            testCase.verifyTrue(isa(zero, 'MTKObject'));            
            testCase.verifyEqual(zero.Scenario, setting);
        end
        
        function get_id(testCase)
            setting = AlgebraicScenario(2);
            id = setting.get([]);
            testCase.verifyTrue(isa(id, 'MTKMonomial'));
            testCase.verifyEqual(id.Operators, uint64.empty(1,0));
            testCase.verifyEqual(id.Coefficient, 1.0);
        end
        
        function get_x(testCase)
            setting = AlgebraicScenario(2);
            x = setting.get([1]);
            testCase.verifyTrue(isa(x, 'MTKMonomial'));
            testCase.verifyEqual(x.Operators, uint64([1]));
            testCase.verifyEqual(x.Coefficient, 1.0);    
        end
        
        function get_xx(testCase)
            setting = AlgebraicScenario(2);
            xx = setting.get([1, 1]);
            testCase.verifyTrue(isa(xx, 'MTKMonomial'));
            testCase.verifyEqual(xx.Operators, uint64([1, 1]));
            testCase.verifyEqual(xx.Coefficient, 1.0);    
        end
        
        function get_xy(testCase)
            setting = AlgebraicScenario(2);
            xy = setting.get([1, 2]);
            testCase.verifyTrue(isa(xy, 'MTKMonomial'));
            testCase.verifyEqual(xy.Operators, uint64([1, 2]));
            testCase.verifyEqual(xy.Coefficient, 1.0);    
        end
        
        function getAll(testCase)
            setting = AlgebraicScenario(3);
            [x, y, z] = setting.getAll();
            testCase.verifyTrue(isa(x, 'MTKMonomial'));
            testCase.verifyTrue(isa(y, 'MTKMonomial'));
            testCase.verifyTrue(isa(z, 'MTKMonomial'));
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
            testCase.verifyTrue(isa(id, 'MTKMonomial'));
            testCase.verifyEqual(id.Operators, uint64.empty(1,0));
            testCase.verifyEqual(id.Coefficient, 1.0);
        end
        
        function getAll_and_bind(testCase)
            setting = AlgebraicScenario(3);
            [x, y, z] = setting.getAll();
            mm = setting.MomentMatrix(1);
            testCase.verifyTrue(isa(x, 'MTKMonomial'));
            testCase.verifyTrue(isa(y, 'MTKMonomial'));
            testCase.verifyTrue(isa(z, 'MTKMonomial'));
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
end
        