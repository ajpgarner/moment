function [output, matched] = spliceProperty(obj, indices, propertyName)
    switch propertyName
        case 'Constituents'
            if obj.IsScalar
                output = obj.Constituents;
            else                
                output = obj.Constituents(indices{:});                
                % If index resolves to a single object, drop cell:
                if numel(output) == 1
                    output = output{1};
                end
            end
            matched = true;                
        otherwise
            [output, matched] = ...
                spliceProperty@MTKObject(obj, indices, propertyName);
    end
end