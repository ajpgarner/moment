function output = SimplifySymbolCell(obj, input)
 % SIMPLIFY Get canonical form of polynomial symbol cells.
 % All applicable re-write rules will be applied.
 %
 % SYNTAX
 %      1. ouptput_cell = setting.SimplifySymbolCell(input_cell)
 %
 %

    assert(nargin==2 && iscell(input)); 
 	output = mtk('simplify', 'polynomial', obj.System.RefId, input);
 end