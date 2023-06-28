function result = isPropertyMTKObject(obj, property_name)
% ISPROPERTYMTKOBJECT Detect if a property is also an MTK object.
%
% This is needed for correct subsref dot behaviour.
%
    result = false;
end