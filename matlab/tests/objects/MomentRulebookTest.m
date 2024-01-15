classdef MomentRulebookTest < MTKTestBase
%MONOMIALTEST Tests for MTKMomentRulebook.

methods(Test, TestTags={'MTKMonomial', 'MTKObject', 'construct'})
    function create_empty(testCase)
        scenario = AlgebraicScenario(3);
        rulebook = scenario.MomentRulebook();            
        testCase.verifyEqual(numel(rulebook.Polynomials), 0);
    end

    function create_empty_named(testCase)
        scenario = AlgebraicScenario(3);
        rulebook = scenario.MomentRulebook("Empty 1234");
        testCase.verifyEqual(numel(rulebook.Polynomials), 0);
        testCase.verifyEqual(rulebook.Id, uint64(0));
        testCase.verifyEqual(rulebook.Label, "Empty 1234");
    end

    function add_from_symbol_cell(testCase)
        scenario = AlgebraicScenario(3);
        [x, y, z] = scenario.getAll();
        scenario.WordList(1, true);
        poly = - x - y + z;
        neg_poly = -poly;
        testCase.assertTrue(poly.FoundAllSymbols);
        
        rulebook = scenario.MomentRulebook("Test book");
        rulebook.AddFromSymbolCell(poly.SymbolCell);
        testCase.verifyEqual(rulebook.Polynomials(1), neg_poly );
        testCase.verifyEqual(rulebook.SymbolCell, neg_poly.SymbolCell);
    end

    function add_from_op_cell(testCase)
        scenario = AlgebraicScenario(3);
        [x, y, z] = scenario.getAll();
        poly = - x - y + z;
        neg_poly = -poly;
        testCase.assertFalse(poly.FoundAllSymbols);
        
        rulebook = scenario.MomentRulebook("Test book");
        rulebook.AddFromOperatorCell(poly.OperatorCell, true);
        testCase.assertTrue(poly.FoundAllSymbols);
        testCase.verifyEqual(rulebook.Polynomials(1), neg_poly );
        testCase.verifyEqual(rulebook.SymbolCell, neg_poly.SymbolCell);                      
    end

    function add_from_polynomial_existing(testCase)
        scenario = AlgebraicScenario(3);
        [x, y, z] = scenario.getAll();
        poly = - x - y + z;
        neg_poly = -poly;
        scenario.WordList(1, true);
        testCase.assertTrue(poly.FoundAllSymbols);
        
        rulebook = scenario.MomentRulebook("Test book");
        rulebook.AddFromPolynomial(poly);
        testCase.verifyEqual(rulebook.Polynomials(1), neg_poly );
        testCase.verifyEqual(rulebook.SymbolCell, neg_poly.SymbolCell);                      
    end

    function add_from_polynomial_new(testCase)
        scenario = AlgebraicScenario(3);
        [x, y, z] = scenario.getAll();
        poly = - x - y + z;
        neg_poly = -poly;
        testCase.assertFalse(poly.FoundAllSymbols);
        
        rulebook = scenario.MomentRulebook("Test book");
        rulebook.AddFromPolynomial(poly, true);
        testCase.assertTrue(poly.FoundAllSymbols);
        testCase.verifyEqual(rulebook.Polynomials(1), neg_poly );
        testCase.verifyEqual(rulebook.SymbolCell, neg_poly.SymbolCell);                      
    end

    function add_from_polynomial_monomial(testCase)
        scenario = AlgebraicScenario(3);
        [x, ~, ~] = scenario.getAll();
        neg_x = MTKPolynomial(-x);
        testCase.assertFalse(x.FoundAllSymbols);
        
        rulebook = scenario.MomentRulebook("Test book");
        rulebook.AddFromPolynomial(x, true);
        
        testCase.assertTrue(x.FoundAllSymbols);
        testCase.verifyEqual(rulebook.Polynomials(1), neg_x);
        testCase.verifyEqual(rulebook.SymbolCell, neg_x.SymbolCell);                      
    end
    
    function add_from_list_two_arrays(testCase)
        scenario = AlgebraicScenario(3);
        [x, y, z] = scenario.getAll();
        xy = x * y;
        wl = scenario.WordList(2, true);
        
        testCase.assertTrue(wl.FoundAllSymbols);
        sym_ids = [x.SymbolId, xy.SymbolId, z.SymbolId];
        values = [1.0, 2.0 + 3.0i, 0.0];
        
        poly_rules = [1 - x; MTKPolynomial(-z); ((2 + 3i) - (xy))];
        
        rulebook = scenario.MomentRulebook("List book");
        rulebook.AddFromList(sym_ids, values);
        testCase.verifyEqual(rulebook.Polynomials, poly_rules);
    end

    function add_from_list_cell(testCase)
        scenario = AlgebraicScenario(3);
        [x, y, z] = scenario.getAll();
        xy = x * y;
        wl = scenario.WordList(2, true);
        testCase.assertTrue(wl.FoundAllSymbols);
        
        list_cell = {{x.SymbolId, 1.0}, ...
                     {xy.SymbolId, 2.0 + 3.0i}, ...
                     {z.SymbolId, 0.0}};
                
        poly_rules = [1 - x; MTKPolynomial(-z); 2+3i-xy];

        rulebook = scenario.MomentRulebook("List book");
        rulebook.AddFromList(list_cell);                
    end
end

end