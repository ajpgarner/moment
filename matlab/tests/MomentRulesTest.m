classdef MomentRulesTest < MTKTestBase
%CREATEMOMENTRULESTEST Unit tests for moment_rules mex function
    methods (Test)
        function Rewrite_Empty(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, {});
            monolith = mtk('moment_rules', ref_id, rules_id , 'rewrite');
            testCase.verifyEqual(monolith, sparse(eye(7)))
        end
        
         function Rewrite_YtoX(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{3, -1.0}, {2, 0.5}}});
            monolith = mtk('moment_rules', ref_id, rules_id , 'rewrite');
            
            expected = eye(7);
            expected(3,3) = 0;
            expected(3,2) = 0.5;
            testCase.verifyEqual(monolith, sparse(expected));
         end
        
         function Rewrite_YtoXplusConstant(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{3, -1.0}, {2, 0.5}, {1, 3.0}}});
            monolith = mtk('moment_rules', ref_id, rules_id , 'rewrite');
            
            expected = eye(7);
            expected(3,3) = 0;
            expected(3,2) = 0.5;
            expected(3,1) = 3.0;
            testCase.verifyEqual(monolith, sparse(expected));
         end
        
       
         function Rewrite_XYtoXplusConstant(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{5, -1.0}, {2, 1.0}, {1, 3.0}}});
            monolith = mtk('moment_rules', ref_id, rules_id , 'rewrite');
            
            expected = eye(7);
            expected(5,5) = 0;
            expected(5,2) = 1.0;
            expected(5,1) = 3.0;
            expected(7,7) = 0; % Im(XY) = 0 implied second rule!
            testCase.verifyEqual(monolith, sparse(expected));
         end  
                  
       
         function Rewrite_XYtoXplusIY(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{5, -1.0}, {2, 1.0}, {3, 1i*3.0}}});
            monolith = mtk('moment_rules', ref_id, rules_id , 'rewrite');
            
            expected = eye(7);
            expected(5,5) = 0;
            expected(5,2) = 1.0;
            expected(7,7) = 0; 
            expected(7,3) = 3.0; 
            testCase.verifyEqual(monolith, sparse(expected));
         end  
                  
       
         function Rewrite_ReXYtoX(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{5, 0.5}, {5, 0.5, true}, {2, -1.0}}});
            monolith = mtk('moment_rules', ref_id, rules_id , 'rewrite');
            
            expected = eye(7);
            expected(5,5) = 0;
            expected(5,2) = 1.0;
            testCase.verifyEqual(monolith, sparse(expected));
         end  
         
         
         function Rewrite_ImXYtoX(testCase)
            ref_id = mtk('algebraic_matrix_system', 'xy');
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{5, -1i*0.5}, {5, 1i*0.5, true}, {2, -1.0}}});
            monolith = mtk('moment_rules', ref_id, rules_id , 'rewrite');
            
            expected = eye(7);
            expected(7,7) = 0;
            expected(7,2) = 1;
            testCase.verifyEqual(monolith, sparse(expected));
         end
         
        function Homogenous_Empty(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, {});
            monolith = mtk('moment_rules', ref_id, rules_id, 'homogenous');
            testCase.verifyEqual(monolith, sparse(zeros(7)))
        end
        
        function Homogenous_YtoX(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{3, -1.0}, {2, 0.5}}});
            monolith = mtk('moment_rules', ref_id, rules_id, 'homogenous');
            
            expected = zeros(7);
            expected(3,3) = -1.0;
            expected(3,2) = 0.5;
            testCase.verifyEqual(monolith, sparse(expected));
        end
         
         function Homogenous_XYtoXplusIY(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{5, -1.0}, {2, 1.0}, {3, 1i}}});
            monolith = mtk('moment_rules', ref_id, rules_id, 'homogenous');
            
            expected = zeros(7);
            expected(5,5) = -1.0;
            expected(5,2) = 1.0;
            expected(7,7) = -1.0;
            expected(7,3) = 1.0;
            testCase.verifyEqual(monolith, sparse(expected));
         end
         
         function Homogenous_ReXYtoX(testCase)
            ref_id = mtk('algebraic_matrix_system', 2);
            [~] = mtk('word_list', ref_id, 2, 'register_symbols');
            % 1, x, y, xx, xy, yy; only xy has imaginary part.
            rules_id = mtk('create_moment_rules', ref_id, ...
                           'input', 'symbols',...
                           {{{5, 0.5}, {5, 0.5, true}, {2, -1.0}}});
            monolith = mtk('moment_rules', ref_id, rules_id, 'homogenous');
            
            expected = zeros(7);
            expected(5,5) = -1.0;
            expected(5,2) = 1.0;
            testCase.verifyEqual(monolith, sparse(expected));
         end  
    end   
end
