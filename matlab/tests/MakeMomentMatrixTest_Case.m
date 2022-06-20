classdef MakeMomentMatrixTest_Case
    %MAKEMOMENTMATRIXTEST_CASE Subclass, for each make_moment_matrix_test
    
    properties
        expected_sym_matrix;
        expected_seq_matrix;
    end
      
    methods
        function obj = MakeMomentMatrixTest_Case(exp_sym, exp_seq)            
            obj.expected_sym_matrix = exp_sym;
            obj.expected_seq_matrix = exp_seq;
        end
        
        function CallAndVerify(testCase, testObj, params)
            [sym_mat, us_key] = npatk('make_moment_matrix', 'symbols', params{:});
            [seq_mat, us_key2] = npatk('make_moment_matrix', 'sequences', params{:});            
            testObj.verifyEqual(us_key, us_key2);
            testCase.Verify(testObj, sym_mat, seq_mat, us_key);            
        end
            
        function Verify(testCase, testObj, actual_sym, actual_seq, actual_list)
           testObj.verifyEqual(actual_sym, testCase.expected_sym_matrix);
           testObj.verifyEqual(actual_seq, testCase.expected_seq_matrix);
           
           reconstruct_seq = testCase.SymbolsToSequences(actual_sym, ...
                                                         actual_list);
                                                     
           testObj.verifyEqual(reconstruct_seq, actual_seq);
        end
        
        function seq_matrix = SymbolsToSequences(~, symbol_matrix, op_list)
            seq_matrix = string(zeros(size(symbol_matrix)));
            op_index_list = [op_list.symbol];
            for index = 1:numel(seq_matrix)   
                symbol_expr = symbol_matrix(index);
                is_conj = (extractBetween(symbol_expr, ...
                                          strlength(symbol_expr), ...
                                          strlength(symbol_expr)) == '*');
                if is_conj
                    symbol_expr = extractBetween(symbol_expr, 1, ...
                                                 strlength(symbol_expr)-1);
                end
                symbol_id = uint64(str2double(symbol_expr));
    
                look_up = find(op_index_list == symbol_id);
                
                if length(look_up) == 1
                    record = op_list(look_up);
                    if is_conj
                        seq_matrix(index) = record.conjugate;
                    else
                        seq_matrix(index) = record.operators;
                    end
                elseif isempty(look_up)
                    seq_matrix(index) = '>NOT FOUND<';
                else
                    seq_matrix(index) = '>NOT UNIQUE<';
                end
            end
        end
    end
end