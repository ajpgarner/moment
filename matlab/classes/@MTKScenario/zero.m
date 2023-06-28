function val = zero(obj)
% Creates algebraic zero object for this setting.
%
% RETURNS:
%   Newly created MTKZero
%
% See also: ALGEBRAIC.ZERO
    val = MTKMonomial.InitZero(obj, [1 1]);
end