function output = SimplifySymbolCell(obj, input, register_new)
 % SIMPLIFY Get canonical form of polynomial symbol cells.
 % All applicable (operator) rewrite rules will be applied.
 % If register new is set to true, unidentified symbols will be created.
 %
 % SYNTAX
 %      1. output_cell = setting.SimplifySymbolCell(input_cell)
 %      2. output_cell = setting.SimplifySymbolCell(input_cell, true)
 %
 %
  
    % Validate arguments
    assert(nargin>=2 && iscell(input), ...
        "Must supply input argument as cell array.");
    if nargin < 3
        register_new = false;
    else
        assert(islogical(register_new), ...
            "register_new must be true or false.");
    end
   
    args = {'output', 'symbol'};
    if register_new
        args{end+1} = 'register';
    end
       
 	output = mtk('import_polynomial', obj.System.RefId, input, args{:});
 end