classdef MakeSymmetricTest < matlab.unittest.TestCase
    %MAKESYMMETRICTEST Unit tests for make_symmetric function
    
    properties(Constant)
        plain_case = MakeSymmetricTest_Case(...
                    [[1, 1, 3]; [-2, 2, 4]; [2, 4, 5]], ...
                    [[1, 1, -1]; [1, -1, 4]; [-1, 4, 5]], ...
                    [["2", "-1"]; ["3", "-1"]]);
            

        zeros_case = MakeSymmetricTest_Case(...
                        [[1, 2, 0]; [0, 4, 5]; [3, 6, 7]], ...
                        [[1, 0, 0]; [0, 4, 5]; [0, 5, 7]], ...
                        [["2", "0"]; ["3", "0"]; ["6", "5"]]);
                    
        rtz_case = MakeSymmetricTest_Case(...
                                [[1, 2, 0]; [-2, 1, 2]; [3, 4, 5]], ...
                                [[1, 0, 0]; [0, 1, 0]; [0, 0, 5]], ...
                                [["2", "0"]; ["3", "0"]; ["4", "0"]]);
        
        id_case  = MakeSymmetricTest_Case(...
                                [[1,2,3];[2,4,5];[3,5,6]], ...
                                [[1,2,3];[2,4,5];[3,5,6]], ...
                                [[]]);
    end
    
    methods(TestMethodSetup)
        function addNPATKpath(testCase)
            import matlab.unittest.fixtures.PathFixture
            testCase.applyFixture(PathFixture([".."]));
        end
    end
    
    methods(TestMethodTeardown)
        function clearNPATK(testCase)
            clear npatk
        end
    end
      
    methods (Test)
        function Plain_DenseToDense(testCase)
            testCase.plain_case.DenseToDense(testCase);            
        end
        
        function Plain_DenseToSparse(testCase)
            testCase.plain_case.DenseToSparse(testCase);
        end
        
        function Plain_SparseToDense(testCase)
            testCase.plain_case.SparseToDense(testCase);
        end
        
        function Plain_SparseToSparse(testCase)
            testCase.plain_case.SparseToSparse(testCase);
        end
        
        function Plain_StringToDense(testCase)
            testCase.plain_case.StringToDense(testCase);
        end
        
        function Plain_StringToSparse(testCase)
           testCase.plain_case.StringToSparse(testCase);
        end
        
        function Zeros_DenseToDense(testCase)
            testCase.zeros_case.DenseToDense(testCase);            
        end
        
        function Zeros_DenseToSparse(testCase)
            testCase.zeros_case.DenseToSparse(testCase);
        end
        
        function Zeros_SparseToDense(testCase)
            testCase.zeros_case.SparseToDense(testCase);
        end
        
        function Zeros_SparseToSparse(testCase)
            testCase.zeros_case.SparseToSparse(testCase);
        end
        
        function Zeros_StringToDense(testCase)
            testCase.zeros_case.StringToDense(testCase);
        end
        
        function Zeros_StringToSparse(testCase)
           testCase.zeros_case.StringToSparse(testCase);
        end

        function ResolveToZero_DenseToDense(testCase)
            testCase.rtz_case.DenseToDense(testCase);            
        end
        
        function ResolveToZero_DenseToSparse(testCase)
            testCase.rtz_case.DenseToSparse(testCase);
        end
        
        function ResolveToZero_SparseToDense(testCase)
            testCase.rtz_case.SparseToDense(testCase);
        end
        
        function ResolveToZero_SparseToSparse(testCase)
            testCase.rtz_case.SparseToSparse(testCase);
        end
        
        function ResolveToZero_StringToDense(testCase)
            testCase.rtz_case.StringToDense(testCase);
        end
        
        function ResolveToZero_StringToSparse(testCase)
           testCase.rtz_case.StringToSparse(testCase);
        end
        
        function NoChange_DenseToDense(testCase)
            testCase.id_case.DenseToDense(testCase);            
        end
        
        function NoChange_DenseToSparse(testCase)
            testCase.id_case.DenseToSparse(testCase);
        end
        
        function NoChange_SparseToDense(testCase)
            testCase.id_case.SparseToDense(testCase);
        end
        
        function NoChange_SparseToSparse(testCase)
            testCase.id_case.SparseToSparse(testCase);
        end
        
        function NoChange_StringToDense(testCase)
            testCase.id_case.StringToDense(testCase);
        end
        
        function NoChange_StringToSparse(testCase)
           testCase.id_case.StringToSparse(testCase);
        end
    end
    
    methods (Test, TestTags={'Error'})
        function Error_NoInput(testCase)
            function no_in()             
               [~, ~] = npatk('make_symmetric');
            end
            testCase.verifyError(@() no_in(), 'npatk:too_few_inputs');           
        end   
        
          function Error_TooManyInputs(testCase)
            function many_in()             
               [~, ~] = npatk('make_symmetric', ...
                              testCase.plain_case.input_dense, ...
                              testCase.plain_case.input_dense);
            end
            testCase.verifyError(@() many_in(), 'npatk:too_many_inputs');           
        end   
              
        function Error_NoOutput(testCase)
            function no_out()             
               npatk('make_symmetric', testCase.plain_case.input_dense);
            end
            testCase.verifyError(@() no_out(), 'npatk:too_few_outputs');
        end  
                
        function Error_DenseAndSparse(testCase)
            function call_sparse_and_dense()
                [~, ~] = npatk('make_symmetric', 'sparse', 'dense', ...
                               testCase.plain_case.input_dense);
            end
           testCase.verifyError(@() call_sparse_and_dense, ...
                                'npatk:mutex_param');           
        end         
    end
end
