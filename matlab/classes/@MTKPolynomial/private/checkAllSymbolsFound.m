function result = checkAllSymbolsFound(obj)
%CHECKALLSYMBOLSFOUND True if every constituent has found all its symbols.
    result = true;
    if obj.IsScalar
        result = all(obj.Constituents.FoundSymbol(:));
    else
        for idx=1:numel(obj.Constituents)
            element = [obj.Constituents{idx}.FoundSymbol];
            if ~all(element(:))
                result = false;
                return % early exit, not all symbols found.
            end
        end
    end   
end
