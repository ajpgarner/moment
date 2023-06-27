 function val = uminus(this)
% UMINUS Unary minus; inverts (all!) coefficient signs
%
% SYNTAX
%   v = -mono
%
% RETURNS
%   A new MTKMonomial, with the coefficient negated.
%
    val = MTKMonomial.InitForOverwrite(this.Scenario, size(this));
    val.Operators = this.Operators;
    val.Hash = this.Hash;
    val.Coefficient = -this.Coefficient;

   % Propagate symbol information
   val.symbol_id = this.symbol_id;
   val.symbol_conjugated = this.symbol_conjugated;
   val.re_basis_index = this.re_basis_index;
   val.im_basis_index = this.im_basis_index;

end