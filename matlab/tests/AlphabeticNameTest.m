classdef AlphabeticNameTest < NPATKTestBase
    %VERSIONTEST Unit tests for alphabetic_name function
    methods (Test)
        function OneIndex_Lower_1(testCase)            
            actual_string = npatk('alphabetic_name', 'lower', 1);
            testCase.verifyEqual(actual_string, 'a');
        end
        
        function OneIndex_Lower_26(testCase)            
            actual_string = npatk('alphabetic_name', 'lower', 26);
            testCase.verifyEqual(actual_string, 'z');
        end
        
        function OneIndex_Lower_27(testCase)            
            actual_string = npatk('alphabetic_name', 'lower', 27);
            testCase.verifyEqual(actual_string, 'aa');
        end
        
        function OneIndex_Lower_Array(testCase)            
            actual_array = npatk('alphabetic_name', ...
                                  'lower', [1, 26, 27]);
            testCase.verifyEqual(actual_array, ["a", "z", "aa"]);
        end    
        
        function OneIndex_Upper_1(testCase)            
            actual_string = npatk('alphabetic_name', 'upper', 1);
            testCase.verifyEqual(actual_string, 'A');
        end
        
        function OneIndex_Upper_26(testCase)            
            actual_string = npatk('alphabetic_name', 'upper', 26);
            testCase.verifyEqual(actual_string, 'Z');
        end
        
        function OneIndex_Upper_27(testCase)            
            actual_string = npatk('alphabetic_name', 'upper', 27);
            testCase.verifyEqual(actual_string, 'AA');
        end
       
        function OneIndex_Upper_Array(testCase)            
            actual_array = npatk('alphabetic_name', ...
                                  'upper', [1, 26, 27]);
            testCase.verifyEqual(actual_array, ["A", "Z", "AA"]);
        end    
        
        function ZeroIndex_Lower_0(testCase)
            actual_string = npatk('alphabetic_name', 'zero_index', ...
                                  'lower', 0);
            testCase.verifyEqual(actual_string, 'a');
        end
        
        function ZeroIndex_Lower_25(testCase)            
            actual_string = npatk('alphabetic_name', 'zero_index', ...
                                  'lower', 25);
            testCase.verifyEqual(actual_string, 'z');
        end
        
        function ZeroIndex_Lower_26(testCase)            
            actual_string = npatk('alphabetic_name', 'zero_index', ...
                                  'lower', 26);
            testCase.verifyEqual(actual_string, 'aa');
        end
        
        function ZeroIndex_Lower_Array(testCase)            
            actual_array = npatk('alphabetic_name', 'zero_index', ...
                                  'lower', [0, 25, 26]);
            testCase.verifyEqual(actual_array, ["a", "z", "aa"]);
        end      
        
        function ZeroIndex_Upper_0(testCase)            
            actual_string = npatk('alphabetic_name', 'zero_index', ...
                                  'upper', 0);
            testCase.verifyEqual(actual_string, 'A');
        end
        
        function ZeroIndex_Upper_25(testCase)            
            actual_string = npatk('alphabetic_name', 'zero_index', ...
                                  'upper', 25);
            testCase.verifyEqual(actual_string, 'Z');
        end
        
        function ZeroIndex_Upper_26(testCase)            
            actual_string = npatk('alphabetic_name', 'zero_index', ...
                                  'upper', 26);
            testCase.verifyEqual(actual_string, 'AA');
        end     
        
        function ZeroIndex_Upper_Array(testCase)            
            actual_array = npatk('alphabetic_name', 'zero_index', ...
                                  'upper', [0, 25, 26]);
            testCase.verifyEqual(actual_array, ["A", "Z", "AA"]);
        end      
    end
        
    methods (Test, TestTags={'Error'})
        function Error_NoInput(testCase)
            function no_in()             
               [~] = npatk('alphabetic_name');
            end
            testCase.verifyError(@() no_in(), 'npatk:too_few_inputs');           
        end  
        
        function Error_TooManyInputs(testCase)
            function no_in()             
               [~] = npatk('alphabetic_name', 0, 1);
            end
            testCase.verifyError(@() no_in(), 'npatk:too_many_inputs');           
        end  
        
        function Error_ZeroInputOneIndex(testCase)
            function no_in()             
               [~] = npatk('alphabetic_name', 0);
            end
            testCase.verifyError(@() no_in(), 'npatk:bad_param');           
        end  
        function Error_ZeroInputOneIndex_Array(testCase)
            function no_in()             
               [~] = npatk('alphabetic_name', [1, 3, 0]);
            end
            testCase.verifyError(@() no_in(), 'npatk:bad_param');           
        end  
        
        function Error_BadInput(testCase)
            function no_in()             
               [~] = npatk('alphabetic_name', 'fff');
            end
            testCase.verifyError(@() no_in(), 'npatk:bad_param');           
        end  

        
    end
end