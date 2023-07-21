function symbol_cell = makeSymbolCell(obj)
%MAKESYMBOLCELL Attempt to make symbol cell of object.


    if any(obj.symbol_id(:) == -1)
        error("Cannot make symbol cell representation before all symbols are identified.");
    end
            
    symbol_cell = cell(size(obj));
    for idx=1:numel(obj)
        if obj.SymbolConjugated(idx)
             symbol_cell{idx} = ...
                {{obj.SymbolId(idx), obj.Coefficient(idx), true}};
        else
            symbol_cell{idx} = ...
                {{obj.SymbolId(idx), obj.Coefficient(idx)}};
        end
    end
end

