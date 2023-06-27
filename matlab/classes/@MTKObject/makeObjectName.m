 function str = makeObjectName(obj)
%MAKEOBJECTNAME Makes human-readable name for objects.
% Should be overloaded by subclasses.
    dim = num2cell(size(obj));
    str = repelem("ComplexObject", dim{:});       
end