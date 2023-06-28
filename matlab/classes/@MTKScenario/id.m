function val = id(obj)
% ID Creates a monomial object for the identity element.
%
% RETURNS:
%   A monomial element representing identity.
%
% See also: MTKMONOMIAL
    val = MTKMonomial.InitValue(obj, 1.0);
end