function makeSymbolCell(obj)
% MAKESYMBOLCELL Produce cell-array representation of polynomial.

    if obj.IsScalar
        obj.symbol_cell = makeOneSymbolCell(obj.Constituents);
    else
        obj.symbol_cell = cellfun(@makeOneSymbolCell, obj.Constituents, ...
                                  'UniformOutput', false);
    end            
    obj.done_sc = true;
end

%% Private functions
function val = makeOneSymbolCell(constituents)
    % No constituents -> {} 
    if isempty(constituents)
        val = cell(1,0);
        return;
    end

    % Not yet got symbols -> error
    if ~all([constituents.FoundSymbol])
        error(MTKPolynomial.err_missing_symbol);
    end

    % Copy specification of polynomial
    val = cell(1, numel(constituents));
    for idx = 1:numel(constituents)
        val{idx} = {constituents(idx).SymbolId, ...
                    constituents(idx).Coefficient};
        if constituents(idx).SymbolConjugated
            val{idx}{end+1} = true;
        end
    end
end