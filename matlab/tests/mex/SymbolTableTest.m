classdef SymbolTableTest < MTKTestBase
%SYMBOLTABLETESTS Unit tests for symbol_table mex function    

    methods(Access=protected)
        function verifyUpToPermutation(testCase, actual, expected)
            expected = [struct('operators', {"0", "1"}, ...
                'conjugate', {"0", "1"}, ...
                'hermitian', {true, true}), expected];
            
            testCase.assertEqual(length(actual), length(expected))
            
            found = false(1, length(expected));
            
            % Match rows
            for index = 1:length(expected)
                the_index = ...
                    find([actual.operators] == expected(index).operators, 1);
                testCase.assertFalse(isempty(the_index));
                found(the_index) = true;
                
                testCase.verifyEqual(actual(the_index).conjugate, ...
                    expected(index).conjugate);
                testCase.verifyEqual(actual(the_index).hermitian, ...
                    expected(index).hermitian);
                
            end
            testCase.verifyTrue(all(found));
            
            % Find & count real basis
            re_symbs = sort([actual([actual.basis_re] > 0).basis_re]);
            exp_re_count = length(expected)-1;
            testCase.verifyEqual(re_symbs, uint64(1:exp_re_count));
            
            % Find imaginary basis
            im_symbs = sort([actual([actual.basis_im] > 0).basis_im]);
            exp_im_count = nnz([actual.hermitian] == false);
            testCase.verifyEqual(im_symbs, uint64(1:exp_im_count));
        end
    end

    methods (Test)
        function FullTable(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            symbols = mtk('symbol_table', ref_id);
            
            expected = struct('operators', ...
                {"A.a0", "A.b0", "B.a0", "B.b0", ...
                "A.a0;A.b0", "A.a0;B.a0", "A.a0;B.b0", ...
                "A.b0;B.a0", "A.b0;B.b0", "B.a0;B.b0"}, ...
                'conjugate', ...
                {"A.a0", "A.b0", "B.a0", "B.b0", ...
                "A.b0;A.a0", "A.a0;B.a0", "A.a0;B.b0", ...
                "A.b0;B.a0", "A.b0;B.b0", "B.b0;B.a0"}, ...
                'hermitian', ...
                {true, true, true, true, false, true, ...
                true, true, true, false});
            
            testCase.verifyUpToPermutation(symbols, expected);
        end
        
         function BySequence(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            find_1 = mtk('symbol_table', ref_id, [2]);
            find_2 = mtk('symbol_table', ref_id, [3]);
            testCase.verifyEqual(fieldnames(find_1), ...
                {'symbol'; 'operators'; 'conjugated'; 'conjugate'; ...
                'hermitian'; 'basis_re'; 'basis_im'});
            testCase.verifyEqual(fieldnames(find_2), ...
                {'symbol'; 'operators'; 'conjugated'; 'conjugate'; ...
                'hermitian';'basis_re'; 'basis_im'});
            testCase.verifyNotEqual(find_1.symbol, find_2.symbol);
            testCase.verifyNotEqual(find_1.operators, find_2.operators);
            testCase.verifyNotEqual(find_1.conjugate, find_2.conjugate);            
            testCase.verifyNotEqual(find_1.basis_re, find_2.basis_re);
            testCase.verifyTrue(find_1.hermitian);
            testCase.verifyTrue(find_2.hermitian);
            testCase.verifyEqual(find_1.basis_im, uint64(0));
            testCase.verifyEqual(find_2.basis_im, uint64(0));
         end
                
         function ByCell(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            find = mtk('symbol_table', ref_id, {[2], [3]});
                        
            testCase.verifyEqual(fieldnames(find), ...
                {'symbol'; 'operators'; 'conjugated'; 'conjugate'; ...
                'hermitian'; 'basis_re'; 'basis_im'});
           
            testCase.verifyNotEqual(find(1).symbol, find(2).symbol);
            testCase.verifyNotEqual(find(1).operators, find(2).operators);            
            testCase.verifyFalse(find(1).conjugated);
            testCase.verifyFalse(find(2).conjugated);
         end
        
                
         function ByCellMatrix(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            find = mtk('symbol_table', ref_id, {[1], [2]; [3], [4]});
                        
            testCase.verifyEqual(fieldnames(find), ...
                {'symbol'; 'operators'; 'conjugated'; 'conjugate'; ...
                'hermitian'; 'basis_re'; 'basis_im'});            
            testCase.verifyEqual(find(1, 1).operators, "A.a0");
            testCase.verifyEqual(find(1, 2).operators, "A.b0");
            testCase.verifyEqual(find(2, 1).operators, "B.a0");
            testCase.verifyEqual(find(2, 2).operators, "B.b0");
        end
    end
    
    
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                [~] = mtk('symbol_table');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_BadMatrixSystem1(testCase)
            function no_in()
                [~] = mtk('symbol_table', "ugh");
            end
            testCase.verifyError(@() no_in(), 'mtk:could_not_convert');
        end
        
        function Error_BadMatrixSystem2(testCase)
            function no_in()
                [~] = mtk('symbol_table', 10);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_signature');
        end
       
        function Error_BadMatrixSystem3(testCase)
            function no_in()                
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('symbol_table', ref_id+1);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end    
    end
end