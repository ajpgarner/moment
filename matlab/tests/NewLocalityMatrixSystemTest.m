classdef NewLocalityMatrixSystemTest < MTKTestBase
    %NEWLOCALITYMATRIXSYSTEMTESTS Unit tests for new_locality_matrix_system
    % mex function
    
    methods (Test)
        function CHSH(testCase)
            ref_id = mtk('new_locality_matrix_system', 2, 2, 2);
            testCase.verifyGreaterThan(ref_id, 0);
            
            mm = mtk('moment_matrix', ref_id, 1);
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(1));
            testCase.verifyEqual(sys_info.Symbols, uint64(12));
        end
    end
        
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()             
               ref_id = mtk('new_locality_matrix_system');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');           
        end
        
        function Error_BadParties(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 'ff', ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
        end
           
        function Error_BadParties2(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', -1, ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'mtk:negative_value');           
        end
        
        function Error_BadParties3(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', [1, 2], ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
        end
        
        function Error_MissingOperators(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:missing_param');           
        end
        
         function Error_BadMeasurements1(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 0, ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
         end
        
         function Error_BadMeasurements2(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 2, ...
                              'measurements', "ab", ...
                              'outcomes', 3);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');           
         end
        
         function Error_MissingOperators1(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'operators', 3);
            end 
            % should be outcomes, not operators...!
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');
         end
        
          
         function Error_MissingOperators2(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system',  ...
                              'parties', 2);
            end 
            testCase.verifyError(@() bad_in(), 'mtk:missing_param');
         end
        
         function Error_BadOutcomes1(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'outcomes', 0);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');
         end
                
         function Error_BadOutcomes2(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 2, ...
                              'measurements', 2, ...
                              'outcomes', 'fff');
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');
         end
         
                
         function Error_BadOperators1(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 2, ...
                              'outcomes', -3);
            end
            testCase.verifyError(@() bad_in(), 'mtk:negative_value');
         end
                
         function Error_BadOperators2(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', ...
                              'parties', 2, ...
                              'outcomes', "cd");
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');
         end
                    
         function Error_BadInputsMix(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', 5, ...
                              'parties', 2, ...
                              'operators', 3);
            end
            testCase.verifyError(@() bad_in(), 'mtk:bad_param');
         end
                      
         function Error_BadInputs1(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', -1, 1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:negative_value');
         end
                   
         function Error_BadInputs2(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', 1, -1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:negative_value');
         end
         
         function Error_BadInputs3(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', 1, 1, -1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:negative_value');
         end
         
         function Error_TooManyInputs(testCase)
            function bad_in()             
               ref_id = mtk('new_locality_matrix_system', 1, 1, 1, 1, 1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:too_many_inputs');
         end     
    end
end