function val = rescaleMatrix(obj, factor)
%RESCALEMATRIX Multiply this matrix by a scalar.
% This function should be overloaded for specific rescale behaviour -
% default behaviour will be to cast first to MTKMonomial/MTKPolynomial.
    val = degradeAndCall(obj, factor, @times);
end

