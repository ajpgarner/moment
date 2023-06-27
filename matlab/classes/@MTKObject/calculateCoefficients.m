 function [re, im] = calculateCoefficients(obj)
% CALCULATECOEFFICIENTS Overload this with actual calculation.  
%
% The format of the expected output depends on the dimension_type.
% For scalar, outputs should be complex, sparse, column vectors.
%
% For row-vector and col-vector, outputs should be a complex,
% sparse matrices; with each column defining one Scalar value
% (transposes will happen elsewhere in the class for col-vecs).
%
% For matrices and tensors, output should be a cell array of
% complex sparse matrices, with dimension 2 less than that of this
% object.
%
    error(obj.err_cannot_calculate, class(obj));
end