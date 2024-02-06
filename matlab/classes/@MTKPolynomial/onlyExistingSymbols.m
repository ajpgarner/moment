function result = onlyExistingSymbols(obj)
%ONLYEXISTINGSYMBOLS Filter array by found symbols
    
    %  Which symbols do we have?
    filter_mask = obj.FoundSymbol;
    
    % If all symbols, return self
    if all(filter_mask(:))
        result = obj;
        return;
    end
    
    % If no symbols, return empty polynomial
    if ~any(filter_mask(:))
        result = MTKPolynomial.empty(0,0);
    end
    
    % Otherwise, return splice:
    % (Recall: overloaded subsref doesn't work from within a class!)
    mask_subs = struct;
    mask_subs(1).type = '()';
    mask_subs(1).subs = {filter_mask};
    result = subsref(obj, mask_subs);
    
end

