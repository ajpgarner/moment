classdef NewMatrixSystemTest < NPATKTestBase
    
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()             
               ref_id = npatk('new_matrix_system');
            end
            testCase.verifyError(@() no_in(), 'npatk:too_few_inputs');           
        end
        
        function Error_BadParties(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 'ff', ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
        end
           
        function Error_BadParties2(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', -1, ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');           
        end
        
        function Error_BadParties3(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', [1, 2], ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
        end
        
        function Error_MissingOperators(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:missing_param');           
        end
        
         function Error_BadMeasurements1(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 0, ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
         end
        
         function Error_BadMeasurements2(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 2, ...
                              'measurements', "ab", ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');           
         end
        
         function Error_MissingOperators1(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'operators', 3);
            end 
            % should be outcomes, not operators...!
            testCase.verifyError(@() bad_in(), 'npatk:missing_param');
         end
        
          
         function Error_MissingOperators2(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system',  ...
                              'parties', 2);
            end 
            testCase.verifyError(@() bad_in(), 'npatk:missing_param');
         end
        
         function Error_BadOutcomes1(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'outcomes', 0);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
                
         function Error_BadOutcomes2(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'outcomes', 'fff');
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
         
                
         function Error_BadOperators1(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 2, ...
                              'operators', -3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
                
         function Error_BadOperators2(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', ...
                              'parties', 2, ...
                              'operators', "cd");
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
                    
         function Error_BadInputsMix(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', 5, ...
                              'parties', 2, ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'npatk:bad_param');
         end
                      
         function Error_BadInputs1(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', -1, 1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
                   
         function Error_BadInputs2(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', 1, -1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
         
         function Error_BadInputs3(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', 1, 1, -1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:negative_value');
         end
         
         function Error_TooManyInputs(testCase)
            function bad_in()             
               ref_id = npatk('new_matrix_system', 1, 1, 1, 1, 1);
            end
            testCase.verifyError(@() bad_in(), 'npatk:too_many_inputs');
         end     
    end
end