 classdef MakeMomentMatrixTest < NPATKTestBase
    %MAKEMOMENTMATRIXTEST Unit tests for make_moment_matrix function    
    properties(Constant)
        IDOnly = MakeMomentMatrixTest_Case([["1"]], [["1"]]);
        OneOper = MakeMomentMatrixTest_Case([["1", "2"]; ["2", "3"]], ...
                                            [["1", "A.0"]; ["A.0", "A.0;A.0"]]);
                                        
        CHSH = MakeMomentMatrixTest_Case(...
                    [["1", "2", "3", "4", "5"]; ...
                     ["2", "2", "6", "7", "8"]; ...
                     ["3", "6*", "3", "9", "10"]; ...
                     ["4", "7", "9", "4", "11"]; ...
                     ["5", "8", "10", "11*", "5"]], ...
                    [["1", "A.a0", "A.b0", "B.a0", "B.b0"];
                    ["A.a0", "A.a0", "A.a0;A.b0", "A.a0;B.a0", "A.a0;B.b0"]; ...
                    ["A.b0", "A.b0;A.a0", "A.b0", "A.b0;B.a0", "A.b0;B.b0"]; ...
                    ["B.a0", "A.a0;B.a0", "A.b0;B.a0", "B.a0", "B.a0;B.b0"]; ...
                    ["B.b0", "A.a0;B.b0", "A.b0;B.b0", "B.b0;B.a0", "B.b0"]]);
                
        OnePartyTwoMmtLevel2 = MakeMomentMatrixTest_Case(...
            [["1",  "2", "3",  "4*", "4" ]; ...
             ["2",  "2", "4*", "4*", "5" ]; ...
             ["3",  "4", "3",  "6",  "4" ]; ...
             ["4",  "4", "6",  "6",  "7*"]; ...
             ["4*", "5", "4*", "7",  "5"]], ...
            [["1", "A.a0", "A.b0", "A.a0;A.b0", "A.b0;A.a0"]; ...
             ["A.a0", "A.a0", "A.a0;A.b0", "A.a0;A.b0", "A.a0;A.b0;A.a0"]; ...
             ["A.b0", "A.b0;A.a0", "A.b0", "A.b0;A.a0;A.b0", "A.b0;A.a0"]; ...
             ["A.b0;A.a0", "A.b0;A.a0", "A.b0;A.a0;A.b0", "A.b0;A.a0;A.b0", "A.b0;A.a0;A.b0;A.a0"]; ...
             ["A.a0;A.b0", "A.a0;A.b0;A.a0", "A.a0;A.b0", "A.a0;A.b0;A.a0;A.b0", "A.a0;A.b0;A.a0"]]);
  
    end
    
    methods (Test)
        function OneOper_Level0(testCase) 
            testCase.IDOnly.CallAndVerify(testCase, {1, 0});
        end
        
        function OneOper_Level0Params(testCase) 
            testCase.IDOnly.CallAndVerify(testCase, {'operators', 1, 'level', 0});
        end
        
        function OneOper_Level1(testCase) 
            testCase.OneOper.CallAndVerify(testCase, {1, 1});
        end
        
        function OneOper_Level1Params(testCase) 
            testCase.OneOper.CallAndVerify(testCase, {'operators', 1, 'level', 1});
        end
        
        function CHSH_Level1(testCase) 
            testCase.CHSH.CallAndVerify(testCase, {2, 2, 2, 1});
        end
                
        function CHSH_Level1Params(testCase) 
            testCase.CHSH.CallAndVerify(testCase, {'parties', 2, ...
                                                   'measurements', 2, ...
                                                   'outcomes', 2, ...
                                                   'level', 1});
        end
        
                     
        function CHSH_Level1Object(testCase) 
            chsh_setting = Setting(2);
            chsh_setting.Parties(1).AddMeasurement(2);
            chsh_setting.Parties(1).AddMeasurement(2);
            chsh_setting.Parties(2).AddMeasurement(2);
            chsh_setting.Parties(2).AddMeasurement(2);
            testCase.CHSH.CallAndVerify(testCase, {chsh_setting, 1});
        end
                   
        function CHSH_Level1ObjectParams(testCase) 
            chsh_setting = Setting(2);
            chsh_setting.Parties(1).AddMeasurement(2);
            chsh_setting.Parties(1).AddMeasurement(2);
            chsh_setting.Parties(2).AddMeasurement(2);
            chsh_setting.Parties(2).AddMeasurement(2);
            testCase.CHSH.CallAndVerify(testCase, ...
                                        {'setting', chsh_setting, ...
                                         'level', 1});
        end
        
        function SaveAndRestore(testCase)
            chsh_setting = Setting(2);
            chsh_setting.Parties(1).AddMeasurement(2);
            chsh_setting.Parties(1).AddMeasurement(2);
            chsh_setting.Parties(2).AddMeasurement(2);
            chsh_setting.Parties(2).AddMeasurement(2);
            [save_sym, save_key] = npatk('make_moment_matrix', ...
                                         'symbols', chsh_setting, 1);
            [save_seq, ~] = npatk('make_moment_matrix', 'sequences', ...
                                  chsh_setting, 1);
            storage_key = npatk('make_moment_matrix', 'reference', ...
                                chsh_setting, 1);
            [ret_sym, ret_key] = npatk('make_moment_matrix', ...
                                       'reference_id', storage_key);
            [ret_seq, ~] = npatk('make_moment_matrix', 'sequences', ...
                                 'reference_id', storage_key);
            testCase.verifyEqual(ret_sym, save_sym);
            testCase.verifyEqual(ret_key, save_key);
            testCase.verifyEqual(ret_seq, save_seq);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()             
               [~, ~] = npatk('make_moment_matrix');
            end
            testCase.verifyError(@() no_in(), 'npatk:too_few_inputs');           
        end
        
        function Error_BadLevel1(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 'ff', ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
        end
        
        function Error_BadLevel2(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', -1, ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');           
        end
      
        function Error_BadLevel3(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', [1, 2], ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
        end    
          
        function Error_BadLevel4(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', "-1", ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');           
        end
        
        function Error_BadParties(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 'ff', ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
        end
           
        function Error_BadParties2(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', -1, ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');           
        end
        
        function Error_BadParties3(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', [1, 2], ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
        end
        
        function Error_MissingOperators(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:missing_param');           
        end
        
         function Error_BadMeasurements1(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2, ...
                              'measurements', 0, ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
         end
        
         function Error_BadMeasurements2(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2, ...
                              'measurements', "ab", ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
         end
        
         function Error_MissingOperators1(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'operators', 3);
            end 
            % should be outcomes, not operators...!
            testCase.verifyError(@() bad_in(), 'npatk:missing_param');
         end
        
          
         function Error_MissingOperators2(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2);
            end 
            testCase.verifyError(@() bad_in(), 'npatk:missing_param');
         end
        
         function Error_BadOutcomes1(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'outcomes', 0);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
                
         function Error_BadOutcomes2(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'outcomes', 'fff');
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
         
                
         function Error_BadOperators1(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2, ...
                              'operators', -3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
                
         function Error_BadOperators2(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 'level', 1, ...
                              'parties', 2, ...
                              'operators', "cd");
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
                    
         function Error_BadInputsMix(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 5, 'level', 1, ...
                              'parties', 2, ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
                      
         function Error_BadInputs1(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', -1, 1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
                   
         function Error_BadInputs2(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 1, -1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
         
         function Error_BadInputs3(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 1, 1, -1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
         
         function Error_BadInputs4(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 1, 1, 1, -1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end     
        
         function Error_TooManyInputs(testCase)
            function bad_in()             
               [~, ~] = npatk('make_moment_matrix', 1, 1, 1, 1, 1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:too_many_inputs');
         end     
    end
end