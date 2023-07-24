classdef MomentRulebookTest < MTKTestBase
%MONOMIALRULESTESTS Unit tests for MomentRulebook matlab class
    methods (Test, TestTags={'mex'})
        function EmptyRules(testCase)
            setting = AlgebraicScenario(["x", "y"], 'hermitian', true);
            
            rules = MomentRulebook(setting);
            testCase.verifyEqual(rules.Id, uint64(0));
            testCase.verifyEmpty(rules.SymbolCell);
            testCase.verifyEmpty(rules.Polynomials);
            testCase.verifyEmpty(rules.Strings);
        end
        
        function AddPolynomial(testCase)
            setting = AlgebraicScenario(["x", "y", "z"], 'hermitian', true);            
            [x, y, z] = setting.getAll();
            
            rules = MomentRulebook(setting);
            rules.Add(x*y - z); % <xy> = <z>
                                    
            testCase.verifyEqual(rules.Id, uint64(0));
            testCase.verifyEqual(length(rules.SymbolCell), 1);
            testCase.verifyEqual(length(rules.Polynomials), 1);
            testCase.verifyEqual(length(rules.Strings), 1);
            
            expected_rule = {{int64(2), 1.0}, {int64(3), -1.0}};
            testCase.verifyEqual(rules.SymbolCell{1}, expected_rule);            
        end
        
        function AddCellRule(testCase)
            setting = AlgebraicScenario(["x", "y", "z"], 'hermitian', true);            
            rules = MomentRulebook(setting);
            input_rule = {{uint64([3]), 1.0}, {uint64([1, 2]), -1.0}};
            rules.AddFromOperatorCell({input_rule}, true);     
            rule_poly = rules.Polynomials;
            testCase.verifyEqual(rule_poly.OperatorCell, input_rule);            
        end
        
        function AddNonCanonicalCellRule(testCase)
            setting = AlgebraicScenario(["x", "y", "z"], 'hermitian', true);            
            rules = MomentRulebook(setting);
            input_rule = {{uint64([3]), 1.0}, ...
                          {uint64([1, 2]), -1.0}, ...
                          {uint64([3]), 2.0}};
            expected_rule = {{uint64([3]), 3.0}, {uint64([1, 2]), -1.0}};
            rules.AddFromOperatorCell({input_rule}, true);            
            rule_poly = rules.Polynomials;
            testCase.verifyEqual(rule_poly.OperatorCell, expected_rule);
        end
    end
    
end