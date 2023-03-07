classdef GroupTest < MTKTestBase
%GROUPTEST Tests for Symmetry.Group
   
   %% Generation
    methods(Test)
       function Z2(testCase)
           generators = {[0 1; 1 0]};
           group = Symmetry.Group(generators);
           group.Generate();
           testCase.verifyEqual(group.Size, 2);
           testCase.verifyEqual(group.RepDimension, 2);
       end
       
      function S3(testCase)
           generators = {[0 1 0; 1 0 0; 0 0 1], ...
                         [1 0 0; 0 0 1; 0 1 0]};
           group = Symmetry.Group(generators);
           group.Generate();
           testCase.verifyEqual(group.Size, 6);
           testCase.verifyEqual(group.RepDimension, 3);
      end
        
      function S4(testCase)
           generators = {[0 1 0 0; 1 0 0 0; 0 0 1 0; 0 0 0 1], ...
                         [1 0 0 0; 0 0 1 0; 0 1 0 0; 0 0 0 1], ...
                         [1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]};
           group = Symmetry.Group(generators);
           group.Generate();
           testCase.verifyEqual(group.Size, 24);
           testCase.verifyEqual(group.RepDimension, 4);
       end
    end    
end

