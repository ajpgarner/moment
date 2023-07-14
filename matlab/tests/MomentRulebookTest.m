classdef MomentRulebookTest < MTKTestBase
%MONOMIALRULESTESTS Unit tests for MomentRulebook matlab class
    methods (Test)
        function EmptyRules(testCase)
            setting = AlgebraicScenario(["x", "y"], {}, true);
            
            rules = MomentRuleBook(setting);
            testCase.verifyEqual(rules.RuleBookId, uint64(0));
            testCase.verifyEmpty(rules.RawRuleCell);
            testCase.verifyEmpty(rules.RulePolynomials);
            testCase.verifyEmpty(rules.RuleStrings);
        end
        
        function AddPolynomial(testCase)
            setting = AlgebraicScenario(["x", "y", "z"], {}, true);            
            [x, y, z] = setting.getAll();
            
            rules = MomentRuleBook(setting);
            rules.Add(x*y - z); % <xy> = <z>
                                    
            testCase.verifyEqual(rules.RuleBookId, uint64(0));
            testCase.verifyEqual(length(rules.RawRuleCell), 1);
            testCase.verifyEqual(length(rules.RulePolynomials), 1);
            testCase.verifyEqual(length(rules.RuleStrings), 1);
            
            expected_rule = {{uint64([3]), 1.0}, {uint64([1, 2]), -1.0}};
            testCase.verifyEqual(rules.RawRuleCell{1}, expected_rule);            
        end
        
        function AddCellRule(testCase)
            setting = AlgebraicScenario(["x", "y", "z"], {}, true);            
            rules = MomentRuleBook(setting);
            input_rule = {{uint64([3]), 1.0}, {uint64([1, 2]), -1.0}};
            rules.AddFromOperatorSequences({input_rule}, true);            
            testCase.verifyEqual(rules.RawRuleCell{1}, input_rule);            
        end
        
        function AddNonCanonicalCellRule(testCase)
            setting = AlgebraicScenario(["x", "y", "z"], {}, true);            
            rules = MomentRuleBook(setting);
            input_rule = {{uint64([3]), 1.0}, ...
                          {uint64([1, 2]), -1.0}, ...
                          {uint64([3]), 2.0}};
            expected_rule = {{uint64([3]), 3.0}, {uint64([1, 2]), -1.0}};
            rules.AddFromOperatorSequences({input_rule}, true);            
            testCase.verifyEqual(rules.RawRuleCell{1}, expected_rule);            
        end
    end
    
end