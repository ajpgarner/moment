 classdef MomentMatrixTest < MTKTestBase
    %MAKEMOMENTMATRIXTEST Unit tests for make_moment_matrix function    
    properties(Constant)
        IDOnly = MomentMatrixTest_Case([["1"]], [["1"]]);
        OneOper = MomentMatrixTest_Case([["1", "2"]; ["2", "2"]], ...
                                            [["1", "<A.a0>"]; ["<A.a0>", "<A.a0>"]]);
                                        
        CHSH = MomentMatrixTest_Case(...
                    [["1", "2", "3", "4", "5"]; ...
                     ["2", "2", "6", "7", "8"]; ...
                     ["3", "6*", "3", "9", "10"]; ...
                     ["4", "7", "9", "4", "11"]; ...
                     ["5", "8", "10", "11*", "5"]], ...
         [["1", "<A.a0>", "<A.b0>", "<B.a0>", "<B.b0>"];
          ["<A.a0>", "<A.a0>", "<A.a0;A.b0>", "<A.a0;B.a0>", "<A.a0;B.b0>"]; ...
          ["<A.b0>", "<A.b0;A.a0>", "<A.b0>", "<A.b0;B.a0>", "<A.b0;B.b0>"]; ...
          ["<B.a0>", "<A.a0;B.a0>", "<A.b0;B.a0>", "<B.a0>", "<B.a0;B.b0>"]; ...
          ["<B.b0>", "<A.a0;B.b0>", "<A.b0;B.b0>", "<B.b0;B.a0>", "<B.b0>"]]);
                
        OnePartyTwoMmtLevel2 = MomentMatrixTest_Case(...
            [["1",  "2", "3",  "4*", "4" ]; ...
             ["2",  "2", "4*", "4*", "5" ]; ...
             ["3",  "4", "3",  "6",  "4" ]; ...
             ["4",  "4", "6",  "6",  "7*"]; ...
             ["4*", "5", "4*", "7",  "5"]], ...
            [["1", "<A.a0>", "<A.b0>", "<A.a0;A.b0>", "<A.b0;A.a0>"]; ...
             ["<A.a0>", "<A.a0>", "<A.a0;A.b0>", "<A.a0;A.b0>", "<A.a0;A.b0;A.a0>"]; ...
             ["<A.b0>", "<A.b0;A.a0>", "<A.b0>", "<A.b0;A.a0;A.b0>", "<A.b0;A.a0>"]; ...
             ["<A.b0;A.a0>", "<A.b0;A.a0>", "<A.b0;A.a0;A.b0>", "<A.b0;A.a0;A.b0>", "<A.b0;A.a0;A.b0;A.a0>"]; ...
             ["<A.a0;A.b0>", "<A.a0;A.b0;A.a0>", "<A.a0;A.b0>", "<A.a0;A.b0;A.a0;A.b0>", "<A.a0;A.b0;A.a0>"]]);
  
    end
    
    methods (Test, TestTags={'mex'})
        function OneOper_Level0(testCase) 
            testCase.IDOnly.CallAndVerify(testCase, {1, 2}, 0);
        end
        
        function OneOper_Level0Params(testCase) 
            testCase.IDOnly.CallAndVerify(testCase, {'outcomes', 2}, 0);
        end
        
        function OneOper_Level1(testCase) 
            testCase.OneOper.CallAndVerify(testCase, {1, 2}, 1);
        end
        
        function OneOper_Level1Params(testCase) 
            testCase.OneOper.CallAndVerify(testCase, {'outcomes', 2}, 1);
        end
        
        function CHSH_Level1(testCase) 
            testCase.CHSH.CallAndVerify(testCase, {2, 2, 2}, 1);
        end
                
        function CHSH_Level1Params(testCase) 
            testCase.CHSH.CallAndVerify(testCase, {'parties', 2, ...
                                                   'measurements', 2, ...
                                                   'outcomes', 2}, 1);
        end
    end
    
    methods (Test, TestTags={'Mex', 'Error'})
        function Error_NoInputs(testCase)
            function no_in()             
               [~] = mtk('moment_matrix');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');           
        end
        
        function Error_BadLevel1(testCase)
            function bad_in()
               system_id = mtk('locality_matrix_system', 2, 2, 2);
               [~] = mtk('moment_matrix', ...
                              'reference_id', system_id, ...
                              'level', 'ff');
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
        end
        
        function Error_BadLevel2(testCase)
            function bad_in()             
               system_id = mtk('locality_matrix_system', 2, 2, 2);
               [~] = mtk('moment_matrix', ...
                              'reference_id', system_id, ...
                              'level', -1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
        end
      
        function Error_BadLevel3(testCase)
            function bad_in()             
               system_id = mtk('locality_matrix_system', 2, 2, 2);
               [~] = mtk('moment_matrix', ...
                              'reference_id', system_id, ... 
                              'level', [1, 2]);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
        end    
          
        function Error_BadLevel4(testCase)
            function bad_in()        
               system_id = mtk('locality_matrix_system', 2, 2, 2);
               [~] = mtk('moment_matrix', ...
                              'reference_id', system_id, ...
                              'level', "-1");
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
        end
        
        function Error_TooManyInputs(testCase)
            function bad_in()             
               [~] = mtk('moment_matrix', 1, 1, 1, 1, 1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:too_many_inputs');
         end     
    end
end