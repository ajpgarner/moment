function result = isPropertyMTKObject(obj, property_name)
% ISPROPERTYMTKOBJECT Detect if a property is also an MTK object.
%
% This is needed for correct subsref dot behaviour.
%
    switch property_name
        case 'Constituents'
            result = true;
        otherwise
            result = isPropertyMTKObject@MTKObject(obj, property_name);
    end
    
end