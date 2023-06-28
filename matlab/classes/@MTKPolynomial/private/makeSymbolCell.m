function makeSymbolCell(obj)

    if obj.IsScalar
        % No constituents -> {} 
        if isempty(obj.Constituents)
            obj.symbol_cell = cell(1,0);
            return;
        end

        % Not yet got symbols -> error
        if ~all([obj.Constituents.FoundSymbol])
            error(obj.err_missing_symbol);
        end

        obj.symbol_cell = cell(1, length(obj.Constituents));
        for idx = 1:length(obj.Constituents)
            obj.symbol_cell{idx} = {obj.Constituents(idx).SymbolId,...
                                    obj.Constituents(idx).Coefficient};
            if (obj.Constituents(idx).SymbolConjugated)
                obj.symbol_cell{idx}{end+1} = true;
            end
        end
        obj.done_sc = true;
    else
         error("Symbol cell not supported for non-scalar polynomials.");
    end            
end