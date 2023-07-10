 function [re, im] = calculateCoefficients(obj)
% CALCULATECOEFFICIENTS Overload this with actual calculation.  
%
% One must generate a sparse matrix for real and for imaginary parts.
% In each matrix, there must be one column per element of the MTKObject.
%
% That is, scalar MTKObjects produce col-vector coefficient objects;
% (1,n) row-vector and (m, 1) col-vector MTKObjects respectively produce 
% (R, n)  and (R, m) matrices (where R is the number of real symbols in
% the matrix system; resp. I for imaginary).
%
% For a MxN matrix MTKObject, one should generate (R, M*N) / (I, M*N)
% matrices, where the column index corresponds to the col-major number of
% the respective element in the MTKObject.
% 
% For tensors, the same principle applies, where col-major generalizes to
% first-index-major.
%
    error(obj.err_cannot_calculate, class(obj));
end