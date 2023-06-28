function [output, matched] = spliceProperty(obj, indices, propertyName)
    switch propertyName
        case 'Constituents'
            if obj.IsScalar
                output = obj.Constituents;
            else
                output = obj.Constituents(indices{:});
            end
            matched = true;                
        otherwise
            [output, matched] = ...
                spliceProperty@MTKObject(obj, indices, propertyName);
    end
end