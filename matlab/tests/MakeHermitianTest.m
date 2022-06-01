classdef MakeHermitianTest < NPATKTestBase
    %MAKEHERMITIANTEST Unit tests for make_hermitian function    
    properties(Constant)
        NoChange_case = MakeHermitianTest_Case(...
                        [["1","2","3"];["2*","4","5"];["3*","5*","6"]],...
                        [["1","2","3"];["2*","4","5"];["3*","5*","6"]],...
                        [[]]);
                    
        InferReal_case = MakeHermitianTest_Case(...
                        [["1","1","3"];["-2","2","4"];["2","4*","5"]], ...
                        [["1","1","-1"]; ["1","-1","4"];["-1","4*","5"]], ...
                        [["2", "-1"]; ["3", "-1"]]);
                    
        NoComplex_case = MakeHermitianTest_Case(...
                        [["1","2","3"];["2","4","5"];["3","5","6"]],...
                        [["1","2","3"];["2","4","5"];["3","5","6"]],...
                        [[]]);
                    
    end
    
    methods (Test)
        function NoChange(testCase)
            testCase.NoChange_case.StringToString(testCase)
        end      
        
        function NoComplex(testCase)
            testCase.NoComplex_case.StringToString(testCase)
        end      
        
        function InferReal(testCase)
            testCase.InferReal_case.StringToString(testCase)
        end    
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInput(testCase)
            function no_in()             
               [~, ~] = npatk('make_hermitian');
            end
            testCase.verifyError(@() no_in(), 'npatk:too_few_inputs');           
        end
        
        function Error_TooManyInputs(testCase)
            function many_in()             
               [~, ~] = npatk('make_hermitian', ...
                              testCase.NoChange_case.input_string, ...
                              testCase.NoChange_case.input_string);
            end
            testCase.verifyError(@() many_in(), 'npatk:too_many_inputs');           
        end   
        
        function Error_NoOutput(testCase)
            function no_out()             
               npatk('make_hermitian', ...
                   testCase.NoChange_case.input_string);
            end
            testCase.verifyError(@() no_out(), 'npatk:too_few_outputs');
        end  
    end
end
