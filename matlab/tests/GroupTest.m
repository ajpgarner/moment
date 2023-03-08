classdef GroupTest < MTKTestBase
%GROUPTEST Tests for Symmetry.Group
   
   %% Generation
    methods(Test)
       function Z2(testCase)
           generators = {[0 1; 1 0]};
           group = Symmetry.Group(generators);
           group.Generate();
           testCase.verifyEqual(group.Size, uint64(2));
           testCase.verifyEqual(group.RepDimension, uint64(2));
       end
       
      function S3(testCase)
           generators = {[0 1 0; 1 0 0; 0 0 1], ...
                         [1 0 0; 0 0 1; 0 1 0]};
           group = Symmetry.Group(generators);
           group.Generate();
           testCase.verifyEqual(group.Size, uint64(6));
           testCase.verifyEqual(group.RepDimension, uint64(3));
      end
        
      function S4(testCase)
           generators = {[0 1 0 0; 1 0 0 0; 0 0 1 0; 0 0 0 1], ...
                         [1 0 0 0; 0 0 1 0; 0 1 0 0; 0 0 0 1], ...
                         [1 0 0 0; 0 1 0 0; 0 0 0 1; 0 0 1 0]};
           group = Symmetry.Group(generators);
           group.Generate();
           testCase.verifyEqual(group.Size, uint64(24));
           testCase.verifyEqual(group.RepDimension, uint64(4));
       end
    end    
end

