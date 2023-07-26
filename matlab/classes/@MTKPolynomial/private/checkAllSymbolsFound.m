function result = checkAllSymbolsFound(obj)
%CHECKALLSYMBOLSFOUND True if every constituent has found all its symbols.
    result = true;
    if obj.IsScalar
        result = found_or_empty(obj.Constituents);
    else
        results = cellfun(@found_or_empty, obj.Constituents);
        result = all(results(:));        
    end   
end



%% Private functions
function val = found_or_empty(constituents)
    if isempty(constituents)
        val = true;
        return;
    end
    val = constituents.FoundAllSymbols;
end