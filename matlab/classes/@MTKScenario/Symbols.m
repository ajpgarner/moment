function val = Symbols(obj)
% SYMBOLS Get table of defined symbols in the matrix system.
%
% RETURNS:
%   Table object with symbol information. 

    val = struct2table(obj.System.SymbolTable);
end