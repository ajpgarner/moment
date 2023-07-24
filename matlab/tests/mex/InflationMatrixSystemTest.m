classdef InflationMatrixSystemTest < MTKTestBase
    %NEWINFLATEDMATRIXSYSTEMTESTS Unit tests for new_inflation_matrix_system
    % mex function
    
    methods (Test)
        function PairUninflated(testCase)
            ref_id = mtk('inflation_matrix_system', ...
                [2 2], {[ 1 2]}, 1);
            testCase.verifyGreaterThan(ref_id, 0);
            
            mm = mtk('moment_matrix', ref_id, 1);
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(1));
            testCase.verifyEqual(sys_info.Symbols, uint64(5)); %0/1/A/B/AB
        end
        
        function PairInflated(testCase)
            ref_id = mtk('inflation_matrix_system', ...
                [2 2], {[ 1 2]}, 2);
            testCase.verifyGreaterThan(ref_id, 0);
            
            mm = mtk('moment_matrix', ref_id, 1);
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(1));
            testCase.verifyEqual(sys_info.Symbols, uint64(8));
        end
        
        function PairCV(testCase)
            ref_id = mtk('inflation_matrix_system', ...
                [0 0], {[ 1 2]}, 1);
            testCase.verifyGreaterThan(ref_id, 0);
            
            mm = mtk('moment_matrix', ref_id, 1);
            sys_info = mtk('list', 'structured', ref_id);
            testCase.verifyEqual(sys_info.RefId, ref_id);
            testCase.verifyEqual(sys_info.Matrices, uint64(1));
            testCase.verifyEqual(sys_info.Symbols, uint64(7));
        end
    end
        
    methods (Test, TestTags={'Error'})
        function Error_NoInputs(testCase)
            function no_in()
                ref_id = mtk('inflation_matrix_system');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooFewInputs(testCase)
            function no_in()
                ref_id = mtk('inflation_matrix_system', [2 2], {[1 2]});
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
         
        function Error_BadObservable1(testCase)
            function no_in()
                ref_id = mtk('inflation_matrix_system', ...
                            [2 -1], {[1 2]}, 1);
            end
            testCase.verifyError(@() no_in(), 'mtk:negative_value');
        end
        
        function Error_BadObservable2(testCase)
            function no_in()
                ref_id = mtk('inflation_matrix_system', ...
                            ["Bad", "Source"], {[1 2]}, 1);
            end
            testCase.verifyError(@() no_in(), 'mtk:could_not_convert');
        end
         
        function Error_BadSource1(testCase)
            function no_in()
                ref_id = mtk('inflation_matrix_system', ...
                            [2 2], {["Bad"]}, 1);
            end
            testCase.verifyError(@() no_in(), 'mtk:could_not_convert');
        end
      
        function Error_BadSource2(testCase)
            function no_in()
                ref_id = mtk('inflation_matrix_system', ...
                            [2 2], "Bad", 1);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_BadSource3(testCase)
            function no_in()
                ref_id = mtk('inflation_matrix_system', ...
                            [2 2], {[1 3]}, 1);
            end
            testCase.verifyError(@() no_in(), 'mtk:bad_param');
        end
        
        function Error_TooManyInputs(testCase)
            function bad_in()
                ref_id = mtk('inflation_matrix_system', ...
                    [2 2], {[ 1 2]}, 1, 1);
            end
            testCase.verifyError(@() bad_in(), 'mtk:too_many_inputs');
        end
    end
end