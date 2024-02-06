function result = checkAllSymbolsFound(obj)
%CHECKALLSYMBOLSFOUND True if every constituent has found all its symbols.

    result = checkSymbolsFound(obj);    
    if ~obj.IsScalar        
        result = all(result(:));
    end   
end