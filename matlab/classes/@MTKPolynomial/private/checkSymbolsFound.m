function result = checkSymbolsFound(obj)
%CHECKSYMBOLSFOUND Check if each constituent has found all its symbols.

    if obj.IsScalar
        result = found_or_empty(obj.Constituents);
    else
        result = cellfun(@found_or_empty, obj.Constituents);
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