function [output, matched] = spliceProperty(obj, indices, propertyName)
    switch propertyName
        case 'SymbolCell'            
            output = obj.SymbolCell(indices{:});
            matched = true;                
        otherwise
            [output, matched] = ...
                spliceProperty@MTKObject(obj, indices, propertyName);
    end
end